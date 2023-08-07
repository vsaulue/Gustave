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

#include <cstdint>
#include <functional>
#include <ostream>

#include <gustave/hash/Hasher.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    struct BlockPosition {
    public:
        using Coord = std::int64_t;

        [[nodiscard]]
        BlockPosition(Utils::NoInit) {}

        [[nodiscard]]
        BlockPosition(Coord x, Coord y, Coord z)
            : x(x)
            , y(y)
            , z(z)
        {

        }

        [[nodiscard]]
        bool operator==(BlockPosition const&) const = default;

        [[nodiscard]]
        BlockPosition operator+(BlockPosition const& rhs) const {
            return { x + rhs.x, y + rhs.y, z + rhs.z };
        }

        friend std::ostream& operator<<(std::ostream& stream, BlockPosition const& pos) {
            return stream << '{' << pos.x << ", " << pos.y << ", " << pos.z << '}';
        }

        Coord x;
        Coord y;
        Coord z;

        using Hasher = Hash::Hasher<BlockPosition, &BlockPosition::x, &BlockPosition::y, &BlockPosition::z>;
    };
}

template<>
struct std::hash<Gustave::Scenes::CuboidGrid::BlockPosition> : Gustave::Scenes::CuboidGrid::BlockPosition::Hasher {};