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

#pragma once

#include <array>

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbour.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    class IndexNeighbours {
    public:
        using Coord = BlockIndex::Coord;
        using Direction = math3d::BasicDirection;

        using Values = std::array<IndexNeighbour,6>;

        using Iterator = Values::const_iterator;

        [[nodiscard]]
        explicit IndexNeighbours(BlockIndex const& source)
            : values_{ NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT() }
            , source_{ source }
            , count_{ 0 }
        {
            addValue(Direction::plusX());
            addValue(Direction::minusX());
            addValue(Direction::plusY());
            addValue(Direction::minusY());
            addValue(Direction::plusZ());
            addValue(Direction::minusZ());
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
        IndexNeighbour const& operator[](unsigned id) const {
            return values_[id];
        }

        [[nodiscard]]
        unsigned size() const {
            return count_;
        }
    private:
        [[nodiscard]]
        static IndexNeighbour NO_INIT() {
            return IndexNeighbour{ utils::NO_INIT };
        }

        void addValue(Direction direction) {
            if (std::optional<BlockIndex> neighbourId = source_.neighbourAlong(direction)) {
                values_[count_] = { direction, *neighbourId };
                ++count_;
            }
        }

        Values values_;
        BlockIndex const& source_;
        unsigned count_;
    };
}
