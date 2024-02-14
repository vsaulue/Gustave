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

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/utils/Hasher.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    struct BlockIndex {
    public:
        using Coord = std::int64_t;
        using Direction = Math3d::BasicDirection;

        [[nodiscard]]
        explicit BlockIndex(Utils::NoInit) {}

        [[nodiscard]]
        BlockIndex(Coord x, Coord y, Coord z)
            : x(x)
            , y(y)
            , z(z)
        {}

        [[nodiscard]]
        std::optional<BlockIndex> neighbourAlong(Direction direction) const {
            static constexpr Coord maxCoord = std::numeric_limits<Coord>::max();
            static constexpr Coord minCoord = std::numeric_limits<Coord>::min();
            std::optional<BlockIndex> result;
            switch (direction) {
            case Direction::plusX:
                if (x < maxCoord) {
                    result = BlockIndex{ x + 1, y ,z };
                }
                break;
            case Direction::minusX:
                if (x > minCoord) {
                    result = BlockIndex{ x - 1, y, z };
                }
                break;
            case Direction::plusY:
                if (y < maxCoord) {
                    result = BlockIndex{ x, y + 1, z };
                }
                break;
            case Direction::minusY:
                if (y > minCoord) {
                    result = BlockIndex{ x, y - 1, z };
                }
                break;
            case Direction::plusZ:
                if (z < maxCoord) {
                    result = BlockIndex{ x, y, z + 1 };
                }
                break;
            case Direction::minusZ:
                if (z > minCoord) {
                    result = BlockIndex{ x, y, z - 1 };
                }
                break;
            }
            return result;
        }

        [[nodiscard]]
        bool operator==(BlockIndex const&) const = default;

        [[nodiscard]]
        BlockIndex operator+(BlockIndex const& rhs) const {
            return { x + rhs.x, y + rhs.y, z + rhs.z };
        }

        friend std::ostream& operator<<(std::ostream& stream, BlockIndex const& index) {
            return stream << '{' << index.x << ", " << index.y << ", " << index.z << '}';
        }

        Coord x;
        Coord y;
        Coord z;

        using Hasher = Utils::Hasher<BlockIndex, &BlockIndex::x, &BlockIndex::y, &BlockIndex::z>;
    };
}

template<>
struct std::hash<Gustave::Scenes::CuboidGrid::BlockIndex> : Gustave::Scenes::CuboidGrid::BlockIndex::Hasher {};
