/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <gustave/scenes/cuboidGridScene/detail/InternalLinks.hpp>
#include <gustave/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGridScene/detail/SceneUpdater.hpp>

#include <TestHelpers.hpp>

namespace cuboid = ::gustave::scenes::cuboidGridScene;

using InternalLinks = cuboid::detail::InternalLinks<libCfg>;
using SceneData = cuboid::detail::SceneData<libCfg>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg>;

using BlockIndex = SceneData::Blocks::BlockIndex;
using ConstBlockDataReference = InternalLinks::ConstBlockDataReference;
using Direction = InternalLinks::Direction;
using Transaction = SceneUpdater::Transaction;
using Value = InternalLinks::Value;

TEST_CASE("scenes::cuboidGridScene::detail::InternalLinks") {
    SceneData scene{ vector3(1.f, 2.f, 3.f, u.length) };

    Transaction t;
    t.addBlock({ {2,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {3,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,3,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,2,3}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {2,2,4}, concrete_20m, 1000.f * u.mass, true });
    SceneUpdater{ scene }.runTransaction(t);

    auto getBlockData = [&](BlockIndex const& index) {
        return scene.blocks.find(index);
    };

    SECTION(".begin() // & .end() ") {
        SECTION("empty") {
            InternalLinks links{ scene, {2,2,3} };
            CHECK(links.begin() == links.end());
        }

        SECTION("non-empty") {
            InternalLinks links{ scene, {2,2,2} };
            std::vector<Value> expected = {
                Value{ getBlockData({3,2,2}), Direction::plusX() },
                Value{ getBlockData({2,3,2}), Direction::plusY() },
                Value{ getBlockData({2,2,3}), Direction::plusZ() },
            };
            CHECK_THAT(links, matchers::c2::RangeEquals(expected));
        }
    }

    SECTION(".size()") {
        SECTION("// 0") {
            InternalLinks links{ scene, {2,2,3} };
            CHECK(links.size() == 0);
        }

        SECTION("// 3") {
            InternalLinks links{ scene, {2,2,2} };
            CHECK(links.size() == 3);
        }
    }
}
