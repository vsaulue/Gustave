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

#include <gustave/core/scenes/cuboidGridScene/detail/SceneBlocks.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

using SceneBlocks = gustave::core::scenes::cuboidGridScene::detail::SceneBlocks<libCfg, SceneUserData>;

using BlockData = SceneBlocks::BlockData;
using BlockIndex = SceneBlocks::BlockIndex;
using Direction = SceneBlocks::Direction;

TEST_CASE("core::scenes::cuboidGridScene::detail::SceneBlocks") {
    auto sceneBlocks = SceneBlocks{ vector3(1.f, 2.f, 3.f, u.length) };
    auto& b1 = sceneBlocks.insert({ {2,3,4}, concrete_20m, 10.f * u.mass, true });
    auto& b2 = sceneBlocks.insert({ {4,6,9}, concrete_20m, 25.f * u.mass, false });

    SECTION(".at()") {
        SECTION("// valid") {
            auto const& res = sceneBlocks.at({ 4,6,9 });
            CHECK(&res == &b2);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(sceneBlocks.at({ 0,0,0 }), std::out_of_range);
        }
    }

    SECTION(".contactAreaAlong(BasicDirection)") {
        CHECK(sceneBlocks.contactAreaAlong(Direction::minusX()) == 6.f * u.area);
        CHECK(sceneBlocks.contactAreaAlong(Direction::plusX()) == 6.f * u.area);
        CHECK(sceneBlocks.contactAreaAlong(Direction::minusY()) == 3.f * u.area);
        CHECK(sceneBlocks.contactAreaAlong(Direction::plusY()) == 3.f * u.area);
        CHECK(sceneBlocks.contactAreaAlong(Direction::minusZ()) == 2.f * u.area);
        CHECK(sceneBlocks.contactAreaAlong(Direction::plusZ()) == 2.f * u.area);
    }

    SECTION(".contains(BlockIndex const&)") {
        SECTION("// true") {
            CHECK(sceneBlocks.contains({ 2,3,4 }));
            CHECK(sceneBlocks.contains({ 4,6,9 }));
        }

        SECTION("// false") {
            CHECK_FALSE(sceneBlocks.contains({ 3,3,4 }));
            CHECK_FALSE(sceneBlocks.contains({ 4,6,8 }));
        }
    }

    SECTION(".erase(BlockIndex const&)") {
        SECTION("// true") {
            bool deleted = sceneBlocks.erase({ 2,3,4 });
            CHECK(deleted);
        }

        SECTION("// false") {
            bool deleted = sceneBlocks.erase({ 2,2,4 });
            CHECK_FALSE(deleted);
        }
    }

    SECTION(".find(BlockIndex const&)") {
        SECTION("// true") {
            auto f1 = sceneBlocks.find({ 2,3,4 });
            CHECK(f1 == &b1);

            auto f2 = sceneBlocks.find({ 4,6,9 });
            CHECK(f2 == &b2);
        }

        SECTION("// false") {
            CHECK_FALSE(sceneBlocks.find({ 2,3,5 }));
        }
    }

    SECTION(".insert(BlockConstructionInfo const&)") {
        CHECK(b1.index() == BlockIndex{2, 3, 4});
        CHECK(b1.mass() == 10.f * u.mass);
        CHECK(b1.isFoundation() == true);

        CHECK(b2.index() == BlockIndex{4, 6, 9});
        CHECK(b2.mass() == 25.f * u.mass);
        CHECK(b2.isFoundation() == false);
    }

    SECTION(".thicknessAlong(BasicDirection)") {
        CHECK(sceneBlocks.thicknessAlong(Direction::minusX()) == 1.f * u.length);
        CHECK(sceneBlocks.thicknessAlong(Direction::plusX()) == 1.f * u.length);
        CHECK(sceneBlocks.thicknessAlong(Direction::minusY()) == 2.f * u.length);
        CHECK(sceneBlocks.thicknessAlong(Direction::plusY()) == 2.f * u.length);
        CHECK(sceneBlocks.thicknessAlong(Direction::minusZ()) == 3.f * u.length);
        CHECK(sceneBlocks.thicknessAlong(Direction::plusZ()) == 3.f * u.length);
    }
}
