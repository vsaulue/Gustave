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

#include <catch2/catch_test_macros.hpp>

#include <gustave/utils/Hasher.hpp>
#include <Point.hpp>

namespace utils = gustave::utils;

TEST_CASE("utils::Hash") {
    SECTION("::operator()()") {
        SECTION("// all fields") {
            constexpr auto fullHasher = utils::Hasher<Point, &Point::x, &Point::y, &Point::z>{};
            CHECK(fullHasher({ 2,4,7 }) == 0x6b0e1ecbcccca9f);
            CHECK(fullHasher({ 2,4,7 }) != fullHasher({ 3,4,7 }));
            CHECK(fullHasher({ 2,4,7 }) != fullHasher({ 2,5,7 }));
            CHECK(fullHasher({ 2,4,7 }) != fullHasher({ 2,4,8 }));
        }

        SECTION("// partial fields") {
            constexpr auto yzHasher = utils::Hasher<Point, &Point::y, &Point::z>{};
            CHECK(yzHasher({ 2,4,7 }) == 0x94709e599e4f391b);
            CHECK(yzHasher({ 2,4,7 }) == yzHasher({ 3,4,7 }));
            CHECK(yzHasher({ 2,4,7 }) != yzHasher({ 2,5,7 }));
        }
    }
}
