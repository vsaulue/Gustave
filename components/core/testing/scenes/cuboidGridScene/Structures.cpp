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

#include <ranges>

#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/Structures.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

namespace cuboid = ::gustave::core::scenes::cuboidGridScene;

template<bool isMutable>
using Structures = cuboid::Structures<libCfg, SceneUserData, isMutable>;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;
using Transaction = SceneUpdater::Transaction;

using BlockIndex = cuboid::BlockIndex;
using StructureIndex = Structures<false>::StructureIndex;

static_assert(std::ranges::forward_range<Structures<false>>);
static_assert(std::ranges::forward_range<Structures<true>>);

TEST_CASE("core::scenes::cuboidGridScene::Structures") {
    Vector3<u.length> const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneData sceneData{ blockSize };
    Structures<true> mStructs{ sceneData };
    auto const& cmStructs = mStructs;
    Structures<false> iStructs{ sceneData };

    Transaction t;
    t.addBlock({ {0,1,0}, concrete_20m, 1000.0f * u.mass, true });
    t.addBlock({ {0,2,0}, concrete_20m, 2000.0f * u.mass, false });
    t.addBlock({ {5,1,0}, concrete_20m, 51000.0f * u.mass, true });
    t.addBlock({ {5,2,0}, concrete_20m, 52000.0f * u.mass, false });
    SceneUpdater{ sceneData }.runTransaction(t);

    auto structureIdOf = [&](BlockIndex const& blockId) {
        auto const blockDataRef = sceneData.blocks.find(blockId);
        REQUIRE(blockDataRef);
        return blockDataRef.structureId();
    };

    SECTION(".at()") {
        auto runValidTest = [&](auto&& structs, bool expectConst) {
            auto const blockId = BlockIndex{ 0,2,0 };
            auto const structId = structureIdOf(blockId);
            auto result = structs.at(structId);
            REQUIRE(result.isValid());
            CHECK(result.blocks().contains(blockId));
            CHECK(expectConst == result.userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mStructs, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmStructs, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iStructs, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(mStructs.at(10), std::out_of_range);
            CHECK_THROWS_AS(cmStructs.at(10), std::out_of_range);
            CHECK_THROWS_AS(iStructs.at(10), std::out_of_range);
        }
    }

    SECTION(".begin() // & end()") {
        auto runTest = [&](auto&& structs, bool expectedConst) {
            auto const expectedIds = std::views::iota(0, 2);
            auto ids = structs | std::views::transform([](auto&& s) { return s.index(); });
            CHECK_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
            CHECK(expectedConst == structs.begin()->userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mStructs, false);
        }

        SECTION("// const") {
            runTest(cmStructs, true);
        }

        SECTION("// immutable") {
            runTest(iStructs, true);
        }
    }

    SECTION(".contains(StructReference const&)") {
        auto const structRef = *mStructs.begin();
        CHECK(mStructs.contains(structRef));

        t.clear();
        t.removeBlock({ 0,2,0 });
        t.removeBlock({ 5,2,0 });
        SceneUpdater{ sceneData }.runTransaction(t);

        CHECK_FALSE(mStructs.contains(structRef));
    }

    SECTION(".find()") {
        auto runValidTest = [&](auto&& structs, bool expectedConst) {
            auto const blockId = BlockIndex{ 0,2,0 };
            auto const structId = structureIdOf(blockId);
            auto result = structs.find(structId);
            REQUIRE(result.isValid());
            CHECK(result.blocks().contains(blockId));
            CHECK(expectedConst == result.userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mStructs, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmStructs, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iStructs, true);
        }

        SECTION("// invalid") {
            auto runInvalidTest = [](auto&& structs) {
                auto structRef = structs.find(10);
                CHECK_FALSE(structRef.isValid());
            };
            runInvalidTest(mStructs);
            runInvalidTest(cmStructs);
            runInvalidTest(iStructs);
        }
    }

    SECTION(".size()") {
        CHECK(mStructs.size() == 2);
    }
}
