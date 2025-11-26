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

#include <gustave/core/scenes/cuboidGridScene/blockReference/Structures.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

template<bool mut>
using Structures = cuboid::blockReference::Structures<libCfg, SceneUserData, mut>;

using BlockIndex = SceneData::Blocks::BlockIndex;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<Structures<true>>);
static_assert(std::ranges::forward_range<Structures<false>>);

TEST_CASE("core::scenes::cuboidGridScene::blockReference::Structures") {
    auto const blockSize = vector3(1.f, 1.f, 1.f, u.length);
    SceneData scene{ blockSize };

    Transaction t;
    auto newBlock = [&](BlockIndex const& index, bool isFoundation) -> void {
        t.addBlock({ index, concrete_20m, 1000.f * u.mass, isFoundation });
    };
    newBlock({ 0,0,0 }, true);
    newBlock({ 0,1,0 }, false);
    newBlock({ 0,2,0 }, false);
    newBlock({ 0,0,1 }, false);
    newBlock({ 0,0,2 }, false);
    newBlock({ 0,0,3 }, true);
    newBlock({ 0,0,4 }, false);
    newBlock({ 0,0,9 }, true);
    SceneUpdater{ scene }.runTransaction(t);

    auto mStructs010 = Structures<true>{ scene, scene.blocks.at({0,1,0}) };
    auto const& cmStructs010 = mStructs010;
    auto iStructs010 = Structures<false>{ scene, scene.blocks.at({0,1,0}) };

    auto mStructs003 = Structures<true>{ scene, scene.blocks.at({0,0,3}) };
    auto const& cmStructs003 = mStructs003;
    auto iStructs003 = Structures<false>{ scene, scene.blocks.at({0,0,3}) };

    auto iStructs009 = Structures<false>{ scene, scene.blocks.at({0,0,9}) };

    auto structIdOf = [&](BlockIndex const& blockId) {
        return scene.blocks.at(blockId).structureId();
    };

    SECTION(".operator[]()") {
        auto runTest = [&](auto&& structs, bool expectedConst) {
            auto&& v0 = structs[0];
            auto&& v1 = structs[1];
            auto const expectedIds = std::array{
                structIdOf({0,0,1}),
                structIdOf({0,0,4})
            };
            auto const ids = std::array{ v0.index(), v1.index() };
            REQUIRE_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
            CHECK(expectedConst == v0.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mStructs003, false);
        }

        SECTION("// const") {
            runTest(cmStructs003, true);
        }

        SECTION("// immutable") {
            runTest(iStructs003, true);
        }
    }

    SECTION(".begin() // && .end()") {
        SECTION("// non-foundation") {
            auto runTest = [&](auto&& structs, bool expectedConst) {
                auto const expectedIds = std::array{
                    structIdOf({0,0,1}),
                    structIdOf({0,0,4})
                };
                auto ids = structs | std::views::transform([](auto&& s) { return s.index(); });
                REQUIRE_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
                CHECK(expectedConst == structs.begin()->userData().isCalledAsConst());
            };

            SECTION("// mutable") {
                runTest(mStructs003, false);
            }

            SECTION("// const") {
                runTest(cmStructs003, true);
            }

            SECTION("// immutable") {
                runTest(iStructs003, true);
            }
        }

        SECTION("// foundation") {
            auto runTest = [&](auto&& structs, bool expectedConst) {
                auto const expectedIds = std::array{
                    structIdOf({0,1,0})
                };
                auto ids = structs | std::views::transform([](auto&& s) { return s.index(); });
                REQUIRE_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
                CHECK(expectedConst == structs.begin()->userData().isCalledAsConst());
            };

            SECTION("// mutable") {
                runTest(mStructs010, false);
            }

            SECTION("// const") {
                runTest(cmStructs010, true);
            }

            SECTION("// immutable") {
                runTest(iStructs010, true);
            }
        }
    }

    SECTION(".size()") {
        CHECK(mStructs003.size() == 2);
        CHECK(cmStructs010.size() == 1);
        CHECK(iStructs003.size() == 2);
    }

    SECTION(".unique()") {
        auto runValidTest = [&](auto&& structs, bool expectedConst) {
            auto& result = structs.unique();
            REQUIRE(result.isValid());
            CHECK(result.index() == structIdOf({ 0,1,0 }));
            CHECK(expectedConst == result.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runValidTest(mStructs010, false);
        }

        SECTION("// const") {
            runValidTest(cmStructs010, true);
        }

        SECTION("// immutable") {
            runValidTest(iStructs010, true);
        }

        SECTION("// invalid - too many structures") {
            CHECK_THROWS_AS(iStructs003.unique(), std::logic_error);
        }

        SECTION("// invalid - no structure") {
            CHECK_THROWS_AS(iStructs009.unique(), std::logic_error);
        }
    }
}
