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

#include <limits>

#include <catch2/catch_test_macros.hpp>

#include <gustave/math/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneNeighbours.hpp>

#include <TestConfig.hpp>

namespace Cuboid = Gustave::Scenes::CuboidGrid;

using BlockPosition = Cuboid::BlockPosition;
using BlockReference = Cuboid::detail::BlockReference<G::libConfig,true>;
using Direction = Gustave::Math::BasicDirection;
using SceneBlocks = Cuboid::detail::SceneBlocks<G::libConfig>;
using SceneNeighbour = Cuboid::detail::SceneNeighbour<G::libConfig,true>;
using SceneNeighbours = Cuboid::detail::SceneNeighbours<G::libConfig,true>;

using Coord = BlockPosition::Coord;
using Limits = std::numeric_limits<Coord>;

static constexpr Coord max{ Limits::max() };
static constexpr Coord min{ Limits::min() };

TEST_CASE("Scene::CuboidGrid::detail::SceneNeighbours") {
    SceneBlocks sceneBlocks;

    auto addBlock = [&](BlockPosition const& position) -> BlockReference {
        return sceneBlocks.insert({ position, concrete_20m, 20.f * u.mass, false });
    };

    BlockReference source = addBlock({ min, 1, 2 });
    BlockReference plusX = addBlock({ 1 + min, 1, 2 });
    BlockReference minusZ = addBlock({ min, 1, 1 });
    addBlock({ max, 1, 2 });
    addBlock({ min, 2, 3 });

    SceneNeighbours neighbours{ sceneBlocks, source.position() };

    std::vector<SceneNeighbour> const expected = {
        {Direction::plusX, plusX},
        {Direction::minusZ, minusZ},
    };

    CHECK_THAT(neighbours, M::C2::RangeEquals(expected));
}
