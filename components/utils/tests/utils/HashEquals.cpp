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

#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include <gustave/utils/HashEquals.hpp>
#include <Point.hpp>

namespace Utils = Gustave::Utils;

TEST_CASE("HashEquals") {
    SECTION("// full HashEquals") {
        using HE = Utils::HashEquals<Point, &Point::x, &Point::y, &Point::z>;
        HE::Set set;
        set.insert({ 0,0,0 });
        set.insert({ 1,0,0 });
        set.insert({ 0,1,0 });
        set.insert({ 0,0,1 });

        CHECK(set.size() == 4);
        CHECK(set.contains({ 1,0,0 }));
        CHECK_FALSE(set.contains({ 2,0,0 }));
    }

    SECTION("// partial HashEquals") {
        using xyHE = Utils::HashEquals<Point, &Point::x, &Point::y>;
        xyHE::Set set;
        set.insert({ 0,0,0 });
        set.insert({ 1,0,0 });
        set.insert({ 0,1,0 });
        auto lastInsert = set.insert({ 0,0,1 });

        CHECK_FALSE(lastInsert.second);
        CHECK(set.size() == 3);
        CHECK(set.contains({ 0,0,15 }));
        CHECK_FALSE(set.contains({ 2,0,0 }));
    }
}
