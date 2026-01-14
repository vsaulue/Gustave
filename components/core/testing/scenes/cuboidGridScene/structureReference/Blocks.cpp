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

#include <gustave/core/scenes/cuboidGridScene/structureReference/Blocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <TestHelpers.hpp>
#include <SceneUserData.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

template<bool mut>
using Blocks = cuboid::structureReference::Blocks<libCfg, SceneUserData, mut>;

using BlockIndex = SceneData::BlockIndex;
using StructureData = SceneData::StructureData;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<Blocks<true>>);
static_assert(std::ranges::forward_range<Blocks<false>>);

TEST_CASE("core::scenes::cuboidGridScene::structureReference::Blocks") {
    auto const blockSize = vector3(1.f, 1.f, 1.f, u.length);
    SceneData scene{ blockSize };

    Transaction t;
    t.addBlock({ {0,0,0}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {0,1,0}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {0,2,0}, concrete_20m, 3000.f * u.mass, false });
    t.addBlock({ {5,0,0}, concrete_20m, 4000.f * u.mass, true });
    t.addBlock({ {5,1,0}, concrete_20m, 5000.f * u.mass, false });
    SceneUpdater{ scene }.runTransaction(t);

    auto structDataOf = [&](BlockIndex const& index) -> StructureData& {
        return scene.structures.at(scene.blocks.at(index).structureId());
    };

    auto mBlocks010 = Blocks<true>{ structDataOf({0,1,0}) };
    auto const& cmBlocks010 = mBlocks010;
    auto iBlocks010 = Blocks<false>{ structDataOf({0,1,0}) };

    SECTION(".at()") {
        auto runValidTest = [&](auto&& blocks, bool expectedConst) {
            auto result = blocks.at({ 0,1,0 });
            REQUIRE(result.isValid());
            CHECK(result.mass() == 2000.f * u.mass);
            CHECK(expectedConst == result.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runValidTest(mBlocks010, false);
        }

        SECTION("// const") {
            runValidTest(cmBlocks010, true);
        }

        SECTION("// immutable") {
            runValidTest(iBlocks010, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(iBlocks010.at({ 5,1,0 }), std::out_of_range);
        }
    }

    SECTION(".begin() // & end()") {
        auto runTest = [&](auto&& blocks, bool expectedConst) {
            auto const expectedIds = std::array{
                BlockIndex{0,0,0},
                BlockIndex{0,1,0},
                BlockIndex{0,2,0},
            };
            auto blockIds = blocks | std::views::transform([](auto&& blockRef) { return blockRef.index(); });
            REQUIRE_THAT(blockIds, matchers::c2::UnorderedRangeEquals(expectedIds));
            auto first = *blocks.begin();
            CHECK(expectedConst == first.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mBlocks010, false);
        }

        SECTION("// const") {
            runTest(cmBlocks010, true);
        }

        SECTION("// immutable") {
            runTest(iBlocks010, true);
        }
    }

    SECTION(".contains()") {
        SECTION("// true") {
            CHECK(iBlocks010.contains({ 0,1,0 }));
        }

        SECTION("// false") {
            CHECK_FALSE(iBlocks010.contains({ 5,1,0 }));
        }
    }

    SECTION(".find()") {
        auto runValidTest = [&](auto&& blocks, bool expectedConst) {
            auto result = blocks.find({ 0,2,0 });
            REQUIRE(result);
            CHECK(result->mass() == 3000.f * u.mass);
            CHECK(expectedConst == result->userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runValidTest(mBlocks010, false);
        }

        SECTION("// const") {
            runValidTest(cmBlocks010, true);
        }

        SECTION("// immutable") {
            runValidTest(iBlocks010, true);
        }

        SECTION("// invalid") {
            auto result = iBlocks010.find({ 5,1,0 });
            CHECK_FALSE(result);
        }
    }

    SECTION(".size()") {
        CHECK(iBlocks010.size() == 3);
    }
}
