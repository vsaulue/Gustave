/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <array>

#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldUpdater.hpp>
#include <gustave/core/worlds/syncWorld/BlockReference.hpp>

#include <TestHelpers.hpp>

using BlockReference = gustave::core::worlds::syncWorld::BlockReference<libCfg>;
using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::core::worlds::syncWorld::detail::WorldUpdater<libCfg>;

using BlockIndex = BlockReference::BlockIndex;
using ContactReference = BlockReference::ContactReference;
using ContactIndex = ContactReference::ContactIndex;
using Direction = ContactIndex::Direction;
using StructureReference = BlockReference::StructureReference;
using Solver = WorldData::Solver;
using StressRatio = BlockReference::StressRatio;
using Transaction = WorldUpdater::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ Solver::Config{ g, solverPrecision } };
    return WorldData{ blockSize, std::move(solver) };
}

static_assert(std::ranges::forward_range<BlockReference::Contacts>);
static_assert(std::ranges::forward_range<BlockReference::Structures>);

TEST_CASE("core::worlds::syncWorld::BlockReference") {
    WorldData world = makeWorld();

    {
        Transaction t;
        t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
        t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,0,2}, concrete_20m, blockMass, true });
        t.addBlock({ {6,6,6}, concrete_20m, blockMass, true });
        t.addBlock({ {7,5,6}, concrete_20m, blockMass, false });
        t.addBlock({ {7,6,6}, concrete_40m, blockMass, false });
        t.addBlock({ {7,7,6}, concrete_20m, blockMass, false });
        WorldUpdater{ world }.runTransaction(t);
    }

    BlockReference b000{ world, {0,0,0} };
    BlockReference b010{ world, {0,1,0} };
    BlockReference b020{ world, {0,2,0} };
    BlockReference b002{ world, {0,0,2} };
    BlockReference b766{ world, {7,6,6} };

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
            CHECK_THAT(b000.contacts(), matchers::c2::UnorderedRangeEquals(expected));
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

    SECTION(".maxPressureStress()") {
        SECTION("// valid") {
            CHECK(b000.maxPressureStress() == concrete_20m);
        }

        SECTION("// invalid") {
            removeBlock({ 0,0,0 });
            CHECK_THROWS_AS(b000.maxPressureStress(), std::out_of_range);
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

    SECTION(".stressRatio()") {
        Real<u.force> const blockWeight = (blockMass * g).norm();
        StressRatio const expected{
            blockWeight / (concrete_40m.compression() * blockSize.x() * blockSize.z()),
            3.f * blockWeight / (concrete_40m.shear() * blockSize.y() * blockSize.z()),
            blockWeight / (concrete_40m.tensile() * blockSize.x() * blockSize.z()),
        };
        StressRatio const result = b766.stressRatio();
        CHECK_THAT(result.compression(), matchers::WithinRel(expected.compression(), solverPrecision));
        CHECK_THAT(result.shear(), matchers::WithinRel(expected.shear(), solverPrecision));
        CHECK_THAT(result.tensile(), matchers::WithinRel(expected.tensile(), solverPrecision));
    }

    SECTION(".structures()") {
        auto structureOf = [&](BlockIndex const& index) {
            return StructureReference{ world.scene.blocks().at(index).structures().unique() };
        };

        SECTION(".begin() // &&.end()") {
            auto const structures = b000.structures();
            auto const expected = std::array{ structureOf({1,0,0}), structureOf({0,1,0}) };
            CHECK_THAT(structures, matchers::c2::UnorderedRangeEquals(expected));
        }

        SECTION(".operator[]") {
            auto const structures = b020.structures();
            auto const first = structures[0];
            REQUIRE(first.isValid());
            CHECK(first == structureOf({ 0,2,0 }));
        }

        SECTION(".unique()") {
            SECTION("// valid") {
                auto const structures = b020.structures();
                auto const result = structures.unique();
                REQUIRE(result.isValid());
                CHECK(result == structureOf({ 0,2,0 }));
            }

            SECTION("// invalid") {
                auto const structures = b002.structures();
                CHECK_THROWS_AS(structures.unique(), std::logic_error);
            }
        }

        SECTION(".size()") {
            auto const structures = b000.structures();
            CHECK(structures.size() == 2);
        }
    }
}
