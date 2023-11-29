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
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbours.hpp>

#include <TestHelpers.hpp>

using BlockPosition = Gustave::Scenes::CuboidGrid::BlockPosition;
using Coord = BlockPosition::Coord;
using Direction = Gustave::Math3d::BasicDirection;
using Limits = std::numeric_limits<Coord>;
using Neighbours = Gustave::Scenes::CuboidGrid::detail::PositionNeighbours;
using PositionNeighbour = Gustave::Scenes::CuboidGrid::detail::PositionNeighbour;

static constexpr Coord max = Limits::max();
static constexpr Coord min = Limits::min();

TEST_CASE("Scene::CuboidGrid::detail::PositionNeighbours") {
    auto runTest = [](BlockPosition const& source, std::vector<PositionNeighbour> const& expected) {
        CHECK_THAT(Neighbours{ source }, M::C2::RangeEquals(expected));
    };

    SECTION("// (min,max,min)") {
        BlockPosition const p = { min,max,min };
        runTest(p, {
            {Direction::plusX,  {min + 1, max,     min}},
            {Direction::minusY, {min    , max - 1, min}},
            {Direction::plusZ,  {min    , max,     min + 1}},
        });
    }

    SECTION("// (max,min,max)") {
        BlockPosition const p = { max,min,max };
        runTest(p, {
            {Direction::minusX, {max - 1, min    , max}},
            {Direction::plusY,  {max    , min + 1, max}},
            {Direction::minusZ, {max    , min    , max - 1}},
        });
    }

    SECTION("// (-1,5,8)") {
        BlockPosition const p = {-3,5,8};
        runTest(p, {
            {Direction::plusX,  {-2,5,8}},
            {Direction::minusX, {-4,5,8}},
            {Direction::plusY,  {-3,6,8}},
            {Direction::minusY, {-3,4,8}},
            {Direction::plusZ,  {-3,5,9}},
            {Direction::minusZ, {-3,5,7}},
        });
    }
}
