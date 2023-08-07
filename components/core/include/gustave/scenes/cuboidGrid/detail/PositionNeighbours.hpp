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

#pragma once

#include <array>

#include <gustave/math/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbour.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    class PositionNeighbours {
    public:
        using Coord = BlockPosition::Coord;
        using Direction = Math::BasicDirection;

        static constexpr Utils::NoInit NO_INIT{};
        using Values = std::array<PositionNeighbour,6>;

        using Iterator = Values::const_iterator;

        [[nodiscard]]
        PositionNeighbours(BlockPosition const& source)
            : values_{ NO_INIT, NO_INIT, NO_INIT, NO_INIT, NO_INIT, NO_INIT }
            , source_{ source }
            , count_{ 0 }
        {
            addValue(!isMax(source.x), Direction::plusX,  {  1, 0, 0 });
            addValue(!isMin(source.x), Direction::minusX, { -1, 0, 0 });
            addValue(!isMax(source.y), Direction::plusY,  {  0, 1, 0 });
            addValue(!isMin(source.y), Direction::minusY, {  0,-1, 0 });
            addValue(!isMax(source.z), Direction::plusZ,  {  0, 0, 1 });
            addValue(!isMin(source.z), Direction::minusZ, {  0, 0,-1 });
        }

        [[nodiscard]]
        Iterator begin() const {
            return values_.begin();
        }

        [[nodiscard]]
        Iterator end() const {
            return values_.begin() + count_;
        }

        [[nodiscard]]
        PositionNeighbour const& operator[](unsigned id) const {
            return values_[id];
        }

        [[nodiscard]]
        unsigned size() const {
            return count_;
        }
    private:
        using Limits = std::numeric_limits<Coord>;

        [[nodiscard]]
        static bool isMin(Coord value) {
            return value == Limits::min();
        }

        [[nodiscard]]
        static bool isMax(Coord value) {
            return value == Limits::max();
        }

        void addValue(bool condition, Direction direction, BlockPosition const& offset) {
            if (condition) {
                values_[count_] = { direction, source_ + offset };
                ++count_;
            }
        }

        Values values_;
        BlockPosition const& source_;
        unsigned count_;
    };
}