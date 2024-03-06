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

#include <gustave/scenes/cuboidGrid/detail/InternalLinks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneUpdater.hpp>

#include <TestHelpers.hpp>

namespace Cuboid = ::Gustave::Scenes::CuboidGrid;

using InternalLinks = Cuboid::detail::InternalLinks<cfg>;
using SceneData = Cuboid::detail::SceneData<cfg>;
using SceneUpdater = Cuboid::detail::SceneUpdater<cfg>;

using BlockIndex = SceneData::Blocks::BlockIndex;
using Direction = InternalLinks::Direction;
using Transaction = SceneUpdater::Transaction;

TEST_CASE("Scene::CuboidGrid::detail::InternalLinks") {
    SceneData scene{ vector3(1.f, 2.f, 3.f, u.length) };

    Transaction t;
    t.addBlock({ {2,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {3,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,3,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,2,3}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {2,2,4}, concrete_20m, 1000.f * u.mass, true });
    SceneUpdater{ scene }.runTransaction(t);

    SECTION(".begin() // & .end() ") {
        SECTION("empty") {
            InternalLinks links{ scene, {2,2,3} };
            CHECK(links.begin() == links.end());
        }

        SECTION("non-empty") {
            InternalLinks links{ scene, {2,2,2} };
            std::vector<Direction> expected = { Direction::plusX(), Direction::plusY(), Direction::plusZ() };
            CHECK_THAT(links, M::C2::RangeEquals(expected));
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
