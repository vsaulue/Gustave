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

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>

#include <TestConfig.hpp>

using SceneBlocks = Gustave::Scenes::CuboidGrid::detail::SceneBlocks<G::libConfig>;
using BlockReference = Gustave::Scenes::CuboidGrid::detail::BlockReference<G::libConfig,true>;
using BlockPosition = Gustave::Scenes::CuboidGrid::BlockPosition;

TEST_CASE("Scene::CuboidGrid::detail::SceneBlocks") {
    SceneBlocks sceneBlocks;
    BlockReference b1 = sceneBlocks.insert({ {2,3,4}, concrete_20m, 10.f * u.mass, true });
    BlockReference b2 = sceneBlocks.insert({ {4,6,9}, concrete_20m, 25.f * u.mass, false });

    SECTION("::contains(BlockPosition const&)") {
        SECTION("// true") {
            CHECK(sceneBlocks.contains({ 2,3,4 }));
            CHECK(sceneBlocks.contains({ 4,6,9 }));
        }

        SECTION("// false") {
            CHECK_FALSE(sceneBlocks.contains({ 3,3,4 }));
            CHECK_FALSE(sceneBlocks.contains({ 4,6,8 }));
        }
    }

    SECTION("::erase(BlockPosition const&)") {
        SECTION("// true") {
            bool deleted = sceneBlocks.erase({ 2,3,4 });
            CHECK(deleted);
        }

        SECTION("// false") {
            bool deleted = sceneBlocks.erase({ 2,2,4 });
            CHECK_FALSE(deleted);
        }
    }

    SECTION("::find(BlockPosition const&)") {
        SECTION("// true") {
            BlockReference f1 = sceneBlocks.find({ 2,3,4 });
            CHECK(f1 == b1);

            BlockReference f2 = sceneBlocks.find({ 4,6,9 });
            CHECK(f2 == b2);
        }

        SECTION("// false") {
            CHECK_FALSE(sceneBlocks.find({ 2,3,5 }));
        }
    }

    SECTION("::insert(BlockConstructionInfo const&)") {
        REQUIRE(b1);
        CHECK(b1.position() == BlockPosition{2, 3, 4});
        CHECK(b1.mass() == 10.f * u.mass);
        CHECK(b1.isFoundation() == true);

        REQUIRE(b2);
        CHECK(b2.position() == BlockPosition{4, 6, 9});
        CHECK(b2.mass() == 25.f * u.mass);
        CHECK(b2.isFoundation() == false);
    }
}
