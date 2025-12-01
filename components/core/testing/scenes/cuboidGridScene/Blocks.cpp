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

#include <array>
#include <ranges>

#include <gustave/core/scenes/cuboidGridScene/Blocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

template<bool mut>
using Blocks = cuboid::Blocks<libCfg, SceneUserData, mut>;

using BlockIndex = Blocks<false>::BlockIndex;
using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<Blocks<false>>);
static_assert(std::ranges::forward_range<Blocks<true>>);

TEST_CASE("core::scenes::cuboidGridScene::Blocks") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    auto scene = SceneData{ blockSize };

    Transaction t;
    t.addBlock({ {0,0,0}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {0,1,0}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {0,2,0}, concrete_20m, 3000.f * u.mass, false });
    SceneUpdater{ scene }.runTransaction(t);

    auto mBlocks = Blocks<true>{ scene };
    auto const& cmBlocks = mBlocks;
    auto iBlocks = Blocks<false>{ scene };

    SECTION(".at()") {
        auto runValidTest = [&](auto&& blocks, bool expectedConst) {
            auto ref = blocks.at({ 0,1,0 });
            REQUIRE(ref.isValid());
            CHECK(ref.mass() == 2000.f * u.mass);
            CHECK(expectedConst == ref.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runValidTest(mBlocks, false);
        }

        SECTION("// const") {
            runValidTest(cmBlocks, true);
        }

        SECTION("// immutable") {
            runValidTest(iBlocks, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(iBlocks.at({ 0,0,8 }), std::out_of_range);
        }
    }

    SECTION(".find()") {
        auto runValidTest = [&](auto&& blocks, bool expectedConst) {
            auto ref = blocks.find({ 0,2,0 });
            REQUIRE(ref.isValid());
            CHECK(ref.mass() == 3000.f * u.mass);
            CHECK(expectedConst == ref.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runValidTest(mBlocks, false);
        }

        SECTION("// const") {
            runValidTest(cmBlocks, true);
        }

        SECTION("// immutable") {
            runValidTest(iBlocks, true);
        }

        SECTION("// invalid") {
            auto ref = iBlocks.find({ 2,2,2 });
            CHECK_FALSE(ref.isValid());
        }
    }

    SECTION(".begin() // & .end()") {
        auto runTest = [&](auto&& blocks, bool expectedConst) {
            auto const expectedIds = std::array{
                BlockIndex{0,0,0}, BlockIndex{0,1,0}, BlockIndex{0,2,0}
            };
            auto ids = blocks | std::views::transform([](auto&& b) { return b.index(); });
            REQUIRE_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
            bool isConst = (*blocks.begin()).userData().isCalledAsConst();
            CHECK(expectedConst == isConst);
        };

        SECTION("// mutable") {
            runTest(mBlocks, false);
        }

        SECTION("// const") {
            runTest(cmBlocks, true);
        }

        SECTION("// immutable") {
            runTest(iBlocks, true);
        }
    }

    SECTION(".size()") {
        CHECK(iBlocks.size() == 3);
    }
}
