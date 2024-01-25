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
#include <gustave/worlds/sync/StructureReference.hpp>

#include <TestHelpers.hpp>

namespace Sync = Gustave::Worlds::Sync;

using WorldData = Sync::detail::WorldData<cfg>;
using WorldUpdater = Sync::detail::WorldUpdater<cfg>;
using BlockReference = Sync::BlockReference<cfg>;
using StructureReference = Sync::StructureReference<cfg>;

using BlockIndex = WorldData::Scene::BlockIndex;
using Solver = WorldData::Solver;
using Transaction = WorldData::Scene::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ std::make_shared<Solver::Config>(g, 1000, solverPrecision) };
    return WorldData{ blockSize, std::move(solver) };
}

static_assert(std::ranges::forward_range<StructureReference::Blocks>);

TEST_CASE("Worlds::Sync::StructureReference") {
    WorldData world = makeWorld();

    {
        Transaction t;
        t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,3,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,4,0}, concrete_20m, blockMass, false });
        WorldUpdater{ world }.runTransaction(t);
    }

    auto structureOf = [&](BlockIndex const& index) {
        auto const sceneBlockStructures = world.scene.blocks().at(index).structures();
        REQUIRE(sceneBlockStructures.size() == 1);
        return StructureReference{ world.structures.at(sceneBlockStructures[0]) };
    };

    auto removeBlock = [&](BlockIndex const& index) {
        Transaction t;
        t.removeBlock(index);
        WorldUpdater{ world }.runTransaction(t);
    };

    StructureReference const s1 = structureOf({ 0,1,0 });
    StructureReference const s4 = structureOf({ 0,4,0 });

    SECTION(".blocks()") {
        SECTION(".at()") {
            SECTION("// valid") {
                BlockReference block = s1.blocks().at({ 0,0,0 });
                CHECK(block == BlockReference{ world, {0,0,0} });
            }

            SECTION("// invalid") {
                CHECK_THROWS_AS(s1.blocks().at({ 0,4,0 }), std::logic_error);
            }
        }

        SECTION(".contains()") {
            CHECK(s1.blocks().contains({ 0,0,0 }));
            CHECK_FALSE(s1.blocks().contains({ 0,4,0 }));
        }

        SECTION(".begin() // & .end()") {
            std::vector<BlockIndex> indices;
            for (auto const& block : s1.blocks()) {
                indices.push_back(block.index());
            }
            std::vector<BlockIndex> expected = { {0,0,0},{0,1,0},{0,2,0},{0,3,0} };
            CHECK_THAT(indices, M::C2::UnorderedRangeEquals(expected));
        }

        SECTION(".find()") {
            SECTION("// valid") {
                auto optBlock = s4.blocks().find({ 0,4,0 });
                REQUIRE(optBlock);
                CHECK(*optBlock == BlockReference{ world, {0,4,0} });
            }

            SECTION("// invalid") {
                auto optBlock = s4.blocks().find({ 0,2,0 });
                CHECK_FALSE(optBlock);
            }
        }

        SECTION(".size()") {
            CHECK(s1.blocks().size() == 4);
        }
    }

    SECTION(".forceVector()") {
        SECTION("// invalid") {
            removeBlock({ 0,3,0 });
            CHECK_FALSE(s1.forceVector({ 0,0,0 }, { 0,1,0 }));
        }

        SECTION("// solved & non-zero contact") {
            auto optForce = s4.forceVector({ 0,3,0 }, { 0,4,0 });
            REQUIRE(optForce);
            CHECK_THAT(*optForce, M::WithinRel(blockMass * g, solverPrecision));
        }

        SECTION("// no contact") {
            auto optForce = s1.forceVector({ 0,0,0 }, { 0,2,0 });
            REQUIRE(optForce);
            CHECK(*optForce == Vector3<u.force>::zero());
        }

        SECTION("// block not in structure") {
            auto opt1 = s1.forceVector({ 0,0,0 }, { 0,4,0 });
            CHECK_FALSE(opt1);
            auto opt2 = s1.forceVector({ 0,4,0 }, { 0,0,0 });
            CHECK_FALSE(opt2);
        }
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(s1.isValid());
        }

        SECTION("// false") {
            removeBlock({ 0,0,0 });
            CHECK_FALSE(s1.isValid());
        }
    }

    SECTION(".state()") {
        SECTION("// solved") {
            CHECK(s1.state() == StructureReference::State::Solved);
        }

        SECTION("// invalid") {
            removeBlock({ 0,0,0 });
            CHECK(s1.state() == StructureReference::State::Invalid);
        }
    }
}
