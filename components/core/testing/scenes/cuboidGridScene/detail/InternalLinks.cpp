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

#include <gustave/core/scenes/cuboidGridScene/detail/InternalLinks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>

#include <TestHelpers.hpp>

namespace cuboid = ::gustave::core::scenes::cuboidGridScene;

using InternalLinks = cuboid::detail::InternalLinks<libCfg,void>;
using SceneData = cuboid::detail::SceneData<libCfg,void>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg,void>;

using BlockIndex = SceneData::BlockIndex;
using Direction = InternalLinks::Direction;
using Transaction = SceneUpdater::Transaction;
using Value = InternalLinks::Value;

TEST_CASE("core::scenes::cuboidGridScene::detail::InternalLinks") {
    auto scene = SceneData{ vector3(1.f, 2.f, 3.f, u.length) };
    auto& blocks = scene.blocks;

    Transaction t;
    t.addBlock({ {2,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {3,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,3,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,2,3}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {2,2,4}, concrete_20m, 1000.f * u.mass, true });
    SceneUpdater{ scene }.runTransaction(t);

    auto links222 = InternalLinks{ scene, {2,2,2} };
    auto links223 = InternalLinks{ scene, {2,2,3} };

    SECTION(".begin() // & .end() ") {
        SECTION("empty") {
            CHECK(links223.begin() == links223.end());
        }

        SECTION("non-empty") {
            auto const expected = std::array{
                Value{ Direction::plusX(), blocks.at({3,2,2}) },
                Value{ Direction::plusY(), blocks.at({2,3,2}) },
                Value{ Direction::plusZ(), blocks.at({2,2,3}) },
            };
            CHECK_THAT(links222, matchers::c2::RangeEquals(expected));
        }
    }

    SECTION(".size()") {
        SECTION("// 0") {
            CHECK(links223.size() == 0);
        }

        SECTION("// 3") {
            CHECK(links222.size() == 3);
        }
    }
}
