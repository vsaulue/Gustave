/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneUpdater.hpp>
#include <gustave/scenes/cuboidGrid/StructureReference.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>

#include <TestHelpers.hpp>

using BlockPosition = Gustave::Scenes::CuboidGrid::BlockPosition;
using BlockReference = Gustave::Scenes::CuboidGrid::BlockReference<cfg>;
using SceneData = Gustave::Scenes::CuboidGrid::detail::SceneData<cfg>;
using SceneUpdater = Gustave::Scenes::CuboidGrid::detail::SceneUpdater<cfg>;
using StructureReference = Gustave::Scenes::CuboidGrid::StructureReference<cfg>;
using Transaction = Gustave::Scenes::CuboidGrid::Transaction<cfg>;

static_assert(std::ranges::forward_range<StructureReference::Blocks>);

TEST_CASE("Scene::CuboidGrid::StructureReference") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneData data{ blockSize };

    {
        Transaction t;
        t.addBlock({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
        t.addBlock({ {2,0,0}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {3,0,0}, concrete_20m, 3000.f * u.mass, false });
        t.addBlock({ {4,0,0}, concrete_20m, 4000.f * u.mass, false });
        SceneUpdater{ data }.runTransaction(t);
    }

    auto structureOf = [&](BlockPosition const& position) -> StructureReference {
        auto const blockDataRef = data.blocks.find(position);
        REQUIRE(blockDataRef);
        auto const rawStructure = blockDataRef.structure();
        REQUIRE(rawStructure);
        auto it = data.structures.find(rawStructure);
        REQUIRE(it != data.structures.end());
        return StructureReference{ *it };
    };

    auto const s1 = structureOf({ 1,0,0 });
    auto const s3 = structureOf({ 3,0,0 });

    SECTION(".blocks()") {
        SECTION(".at()") {
            SECTION("// valid") {
                BlockReference r4 = s3.blocks().at({ 4,0,0 });
                CHECK(r4.mass() == 4000.f * u.mass);
            }

            SECTION("// invalid") {
                auto const blocks = s3.blocks();
                CHECK_THROWS_AS(blocks.at({ 1,0,0 }), std::out_of_range);
            }
        }

        SECTION(".begin() // & end()") {
            std::vector<BlockPosition> blocks;
            for (auto const& block : s1.blocks()) {
                blocks.push_back(block.position());
            }
            std::vector<BlockPosition> expected = { {1,0,0},{2,0,0} };
            CHECK_THAT(blocks, M::C2::UnorderedRangeEquals(expected));
        }

        SECTION(".contains()") {
            SECTION("// true") {
                CHECK(s1.blocks().contains({ 2,0,0 }));
            }

            SECTION("// false") {
                CHECK_FALSE(s3.blocks().contains({ 1,0,0 }));
            }
        }

        SECTION(".find()") {
            SECTION("// valid") {
                auto optBlock = s3.blocks().find({ 2,0,0 });
                REQUIRE(optBlock);
                CHECK(optBlock->isFoundation());
            }

            SECTION("// invalid") {
                auto optBlock = s3.blocks().find({ 1,0,0 });
                CHECK_FALSE(optBlock);
            }
        }

        SECTION(".size()") {
            CHECK(s3.blocks().size() == 3);
        }
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(s1.isValid());
        }

        SECTION("// false") {
            Transaction t;
            t.removeBlock({ 1,0,0 });
            SceneUpdater{ data }.runTransaction(t);
            CHECK_FALSE(s1.isValid());
        }
    }

    SECTION(".solverIndexOf() ") {
        SECTION("// invalid") {
            auto opt = s3.solverIndexOf({ 1,0,0 });
            CHECK_FALSE(opt);
        }

        SECTION("// & .solverStructurePtr()") {
            auto const& solver3 = *s3.solverStructurePtr();
            auto const optIndex2 = s3.solverIndexOf({ 2,0,0 });
            auto const optIndex3 = s3.solverIndexOf({ 3,0,0 });
            auto const optIndex4 = s3.solverIndexOf({ 4,0,0 });
            REQUIRE(optIndex2);
            REQUIRE(optIndex3);
            REQUIRE(optIndex4);
            CHECK(solver3.nodes()[*optIndex2].mass() == 2000.f * u.mass);
            CHECK(solver3.nodes()[*optIndex3].mass() == 3000.f * u.mass);
            CHECK(solver3.nodes()[*optIndex4].mass() == 4000.f * u.mass);
        }
    }
}
