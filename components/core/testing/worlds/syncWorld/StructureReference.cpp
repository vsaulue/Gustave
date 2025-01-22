/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldUpdater.hpp>
#include <gustave/core/worlds/syncWorld/StructureReference.hpp>

#include <TestHelpers.hpp>

using StructureReference = gustave::core::worlds::syncWorld::StructureReference<libCfg>;
using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::core::worlds::syncWorld::detail::WorldUpdater<libCfg>;

using BlockIndex = WorldData::Scene::BlockIndex;
using BlockReference = StructureReference::BlockReference;
using ContactIndex = WorldData::Scene::ContactIndex;
using ContactReference = StructureReference::ContactReference;
using Direction = ContactIndex::Direction;
using Solver = WorldData::Solver;
using Transaction = WorldData::Scene::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ Solver::Config{ g, solverPrecision } };
    return WorldData{ blockSize, std::move(solver) };
}

static_assert(std::ranges::forward_range<StructureReference::Blocks>);

TEST_CASE("core::worlds::syncWorld::StructureReference") {
    WorldData world = makeWorld();

    {
        Transaction t;
        t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,3,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,4,0}, concrete_20m, blockMass, false });
        t.addBlock({ {2,0,2}, concrete_20m, blockMass, false });
        t.addBlock({ {2,1,2}, concrete_20m, blockMass, false });
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

    StructureReference const s010 = structureOf({ 0,1,0 });
    StructureReference const s040 = structureOf({ 0,4,0 });
    StructureReference const s202 = structureOf({ 2,0,2 });

    SECTION(".blocks()") {
        SECTION(".at()") {
            SECTION("// valid") {
                BlockReference block = s010.blocks().at({ 0,0,0 });
                CHECK(block == BlockReference{ world, {0,0,0} });
            }

            SECTION("// invalid") {
                CHECK_THROWS_AS(s010.blocks().at({ 0,4,0 }), std::logic_error);
            }
        }

        SECTION(".contains()") {
            CHECK(s010.blocks().contains({ 0,0,0 }));
            CHECK_FALSE(s010.blocks().contains({ 0,4,0 }));
        }

        SECTION(".begin() // & .end()") {
            std::vector<BlockIndex> indices;
            for (auto const& block : s010.blocks()) {
                indices.push_back(block.index());
            }
            std::vector<BlockIndex> expected = { {0,0,0},{0,1,0},{0,2,0},{0,3,0} };
            CHECK_THAT(indices, matchers::c2::UnorderedRangeEquals(expected));
        }

        SECTION(".find()") {
            SECTION("// valid") {
                auto optBlock = s040.blocks().find({ 0,4,0 });
                REQUIRE(optBlock);
                CHECK(*optBlock == BlockReference{ world, {0,4,0} });
            }

            SECTION("// invalid") {
                auto optBlock = s040.blocks().find({ 0,2,0 });
                CHECK_FALSE(optBlock);
            }
        }

        SECTION(".size()") {
            CHECK(s010.blocks().size() == 4);
        }
    }

    SECTION(".contacts()") {
        SECTION(".at()") {
            SECTION("// valid") {
                auto const id = ContactIndex{ {0,0,0}, Direction::plusY() };
                ContactReference contact = s010.contacts().at(id);
                CHECK(contact == ContactReference{ world, id });
            }

            SECTION("// invalid") {
                auto const id = ContactIndex{ {0,0,0}, Direction::plusY() };
                CHECK_THROWS_AS(s040.contacts().at(id), std::out_of_range);
            }
        }
    }

    SECTION(".forceVector()") {
        SECTION("// invalid contact") {
            removeBlock({ 0,3,0 });
            CHECK_FALSE(s010.forceVector({ 0,0,0 }, { 0,1,0 }));
        }

        SECTION("// unsolved contact") {
            auto optForce = s202.forceVector({ 2,0,2 }, { 2,1,2 });
            CHECK_FALSE(optForce);
        }

        SECTION("// solved & non-zero contact") {
            auto optForce = s040.forceVector({ 0,3,0 }, { 0,4,0 });
            REQUIRE(optForce);
            CHECK_THAT(*optForce, matchers::WithinRel(blockMass * g, solverPrecision));
        }

        SECTION("// no contact") {
            auto optForce = s010.forceVector({ 0,0,0 }, { 0,2,0 });
            REQUIRE(optForce);
            CHECK(*optForce == Vector3<u.force>::zero());
        }

        SECTION("// block not in structure") {
            auto opt1 = s010.forceVector({ 0,0,0 }, { 0,4,0 });
            CHECK_FALSE(opt1);
            auto opt2 = s010.forceVector({ 0,4,0 }, { 0,0,0 });
            CHECK_FALSE(opt2);
        }
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(s010.isValid());
        }

        SECTION("// false") {
            removeBlock({ 0,0,0 });
            CHECK_FALSE(s010.isValid());
        }
    }

    SECTION(".links()") {
        SECTION(".begin() // .end()") {
            std::vector<ContactReference> expected = {
                ContactReference{ world, ContactIndex{ {0,0,0}, Direction::plusY() } },
                ContactReference{ world, ContactIndex{ {0,1,0}, Direction::plusY() } },
                ContactReference{ world, ContactIndex{ {0,2,0}, Direction::plusY() } },
            };
            CHECK_THAT(s010.links(), matchers::c2::UnorderedRangeEquals(expected));
        }
    }

    SECTION(".state()") {
        SECTION("// solved") {
            CHECK(s010.state() == StructureReference::State::Solved);
        }

        SECTION("// invalid") {
            removeBlock({ 0,0,0 });
            CHECK(s010.state() == StructureReference::State::Invalid);
        }
    }
}
