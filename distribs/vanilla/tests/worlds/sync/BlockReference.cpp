/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <catch2/catch_test_macros.hpp>

#include <gustave/worlds/sync/BlockReference.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>
#include <gustave/worlds/sync/detail/WorldUpdater.hpp>

#include <TestHelpers.hpp>

namespace Sync = Gustave::Worlds::Sync;

using BlockReference = Sync::BlockReference<cfg>;
using WorldData = Sync::detail::WorldData<cfg>;
using WorldUpdater = Sync::detail::WorldUpdater<cfg>;

using BlockIndex = BlockReference::BlockIndex;
using ContactReference = BlockReference::ContactReference;
using ContactIndex = ContactReference::ContactIndex;
using Direction = ContactIndex::Direction;
using StructureReference = BlockReference::StructureReference;
using Solver = WorldData::Solver;
using Transaction = WorldUpdater::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ std::make_shared<Solver::Config>(g, 1000, solverPrecision) };
    return WorldData{ blockSize, std::move(solver) };
}

static_assert(std::ranges::forward_range<BlockReference::Contacts>);
static_assert(std::ranges::forward_range<BlockReference::Neighbours>);
static_assert(std::ranges::forward_range<BlockReference::Structures>);

TEST_CASE("Worlds::Sync::BlockReference") {
    WorldData world = makeWorld();

    {
        Transaction t;
        t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
        t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,0,2}, concrete_20m, blockMass, true });
        WorldUpdater{ world }.runTransaction(t);
    }

    BlockReference b000{ world, {0,0,0} };
    BlockReference b010{ world, {0,1,0} };
    BlockReference b020{ world, {0,2,0} };
    BlockReference b002{ world, {0,0,2} };

    auto removeBlock = [&](BlockIndex const& index) {
        Transaction t;
        t.removeBlock(index);
        WorldUpdater{ world }.runTransaction(t);
    };

    SECTION(".contacts()") {
        SECTION("// valid") {
            std::vector<ContactReference> expected = {
                ContactReference{ world, ContactIndex{ {0,0,0}, Direction::plusX() } },
                ContactReference{ world, ContactIndex{ {0,0,0}, Direction::plusY() } },
            };
            CHECK_THAT(b000.contacts(), M::C2::UnorderedRangeEquals(expected));
        }

        SECTION("// invalid") {
            removeBlock(b000.index());
            CHECK_THROWS_AS(b000.contacts(), std::out_of_range);
        }
    }

    SECTION(".index()") {
        removeBlock({ 0,0,0 });
        CHECK(b000.index() == BlockIndex{ 0,0,0 });
        CHECK(b020.index() == BlockIndex{ 0,2,0 });
    }

    SECTION(".isFoundation()") {
        SECTION("// valid") {
            CHECK(b000.isFoundation());
            CHECK_FALSE(b020.isFoundation());
        }

        SECTION("// invalid") {
            removeBlock({ 0,2,0 });
            CHECK_THROWS_AS(b020.isFoundation(), std::out_of_range);
        }
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(b000.isValid());
        }

        SECTION("// false") {
            removeBlock({ 0,0,0 });
            CHECK_FALSE(b000.isValid());
        }
    }

    SECTION(".mass()") {
        SECTION("// valid") {
            CHECK(b000.mass() == blockMass);
        }

        SECTION("// invalid") {
            removeBlock({ 0,0,0 });
            CHECK_THROWS_AS(b000.mass(), std::out_of_range);
        }
    }

    SECTION(".maxStress()") {
        SECTION("// valid") {
            CHECK(b000.maxStress() == concrete_20m);
        }

        SECTION("// invalid") {
            removeBlock({ 0,0,0 });
            CHECK_THROWS_AS(b000.maxStress(), std::out_of_range);
        }
    }

    SECTION(".neighbours()") {
        using Neighbour = BlockReference::Neighbour;

        SECTION("// empty") {
            auto const neighbours = b002.neighbours();
            CHECK(neighbours.begin() == neighbours.end());
        }

        SECTION("// 2 neighbours") {
            auto const neighbours = b010.neighbours();
            std::vector<Neighbour> expected = { Neighbour{b000}, Neighbour{b020} };
            CHECK_THAT(neighbours, M::C2::UnorderedRangeEquals(expected));
        }
    }

    SECTION(".position()") {
        SECTION("// valid") {
            CHECK(b020.position() == vector3(0.f, 4.f, 0.f, u.length));
        }

        SECTION("// invalid") {
            removeBlock({ 0,2,0 });
            CHECK_THROWS_AS(b020.position(), std::out_of_range);
        }
    }

    SECTION(".structures()") {
        auto structureOf = [&](BlockIndex const& index) {
            auto const sceneStructure = world.scene.blocks().at(index).structures()[0];
            return StructureReference{ world.structures.at(sceneStructure) };
        };

        SECTION("// empty") {
            auto const structures = b002.structures();
            CHECK(structures.size() == 0);
            CHECK(structures.begin() == structures.end());
        }

        SECTION("// singleton") {
            auto const structures = b020.structures();
            CHECK(structures.size() == 1);
            CHECK(structures[0] == structureOf({0,2,0}));
        }

        SECTION("// 2 structures") {
            auto const structures = b000.structures();
            CHECK(structures.size() == 2);

            std::vector<StructureReference> expected = { structureOf({1,0,0}), structureOf({0,1,0}) };
            CHECK_THAT(structures, M::C2::UnorderedRangeEquals(expected));
        }
    }
}
