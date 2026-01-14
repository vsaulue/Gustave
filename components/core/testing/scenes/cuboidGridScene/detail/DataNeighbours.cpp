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
#include <limits>
#include <ranges>

#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using DataNeighbours = cuboid::detail::DataNeighbours<libCfg, SceneUserData, true>;
using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;

using BlockConstructionInfo = SceneData::BlockData::BlockConstructionInfo;
using BlockData = SceneData::BlockData;
using BlockIndex = SceneData::BlockIndex;
using DataNeighbour = DataNeighbours::Neighbour;
using Direction = DataNeighbour::Direction;

using Coord = BlockIndex::Coord;
using Limits = std::numeric_limits<Coord>;

static constexpr auto max = Coord{ Limits::max() };
static constexpr auto min = Coord{ Limits::min() };

static_assert(std::ranges::forward_range<DataNeighbours>);

TEST_CASE("core::scenes::cuboidGridScene::detail::DataNeighbours") {
    auto scene = SceneData{ vector3(2.f, 3.f, 1.f, u.length) };

    auto addBlock = [&](BlockIndex const& index) -> BlockData& {
        return scene.blocks.emplace(BlockConstructionInfo{ index, concrete_20m, 20.f * u.mass, false });
    };

    auto& source = addBlock({ min, 1, 2 });
    auto& plusX = addBlock({ 1 + min, 1, 2 });
    auto& minusZ = addBlock({ min, 1, 1 });
    addBlock({ max, 1, 2 });
    addBlock({ min, 2, 3 });

    auto neighbours = DataNeighbours{ scene, source.index() };

    auto const expected = std::array{
        DataNeighbour{ Direction::plusX(), plusX },
        DataNeighbour{ Direction::minusZ(), minusZ },
    };

    CHECK_THAT(neighbours, matchers::c2::RangeEquals(expected));
}
