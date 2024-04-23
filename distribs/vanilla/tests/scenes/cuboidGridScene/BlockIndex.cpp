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

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>

#include <TestConfig.hpp>

using BlockIndex = gustave::core::scenes::cuboidGridScene::BlockIndex;

using Coord = BlockIndex::Coord;
using Direction = BlockIndex::Direction;

TEST_CASE("core::scenes::cuboidGridScene::BlockIndex") {
    static constexpr Coord maxCoord = std::numeric_limits<Coord>::max();
    static constexpr Coord minCoord = std::numeric_limits<Coord>::min();

    SECTION(".neighbourAlong(Direction)") {
        BlockIndex validBlock{ 1, 4, 7 };
        SECTION("// plusX, valid") {
            auto res = validBlock.neighbourAlong(Direction::plusX());
            REQUIRE(res);
            CHECK(*res == BlockIndex{ 2, 4, 7 });
        }

        SECTION("// minusX, valid") {
            auto res = validBlock.neighbourAlong(Direction::minusX());
            REQUIRE(res);
            CHECK(*res == BlockIndex{ 0, 4, 7 });
        }

        SECTION("// plusY, valid") {
            auto res = validBlock.neighbourAlong(Direction::plusY());
            REQUIRE(res);
            CHECK(*res == BlockIndex{ 1, 5, 7 });
        }

        SECTION("// minusY, valid") {
            auto res = validBlock.neighbourAlong(Direction::minusY());
            REQUIRE(res);
            CHECK(*res == BlockIndex{ 1, 3, 7 });
        }

        SECTION("// plusZ, valid") {
            auto res = validBlock.neighbourAlong(Direction::plusZ());
            REQUIRE(res);
            CHECK(*res == BlockIndex{ 1, 4, 8 });
        }

        SECTION("// minusZ, valid") {
            auto res = validBlock.neighbourAlong(Direction::minusZ());
            REQUIRE(res);
            CHECK(*res == BlockIndex{ 1, 4, 6 });
        }

        SECTION("// plusX, overflow") {
            REQUIRE_FALSE(BlockIndex{ maxCoord, 0, 0 }.neighbourAlong(Direction::plusX()));
        }

        SECTION("// minusX, overflow") {
            REQUIRE_FALSE(BlockIndex{ minCoord, 0, 0 }.neighbourAlong(Direction::minusX()));
        }

        SECTION("// plusY, overflow") {
            REQUIRE_FALSE(BlockIndex{ 0, maxCoord, 0 }.neighbourAlong(Direction::plusY()));
        }

        SECTION("// minusY, overflow") {
            REQUIRE_FALSE(BlockIndex{ 0, minCoord, 0 }.neighbourAlong(Direction::minusY()));
        }

        SECTION("// plusZ, overflow") {
            REQUIRE_FALSE(BlockIndex{ 0, 0, maxCoord }.neighbourAlong(Direction::plusZ()));
        }

        SECTION("// minusZ, overflow") {
            REQUIRE_FALSE(BlockIndex{ 0, 0, minCoord }.neighbourAlong(Direction::minusZ()));
        }
    }

    SECTION("::operator+(BlockIndex const&)") {
        BlockIndex lhs = { -1, 4, 7 };
        BlockIndex rhs = { 8, 2, -5 };
        BlockIndex expected = { 7, 6, 2 };
        CHECK(lhs + rhs == expected);
    }
}
