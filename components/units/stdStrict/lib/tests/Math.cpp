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

#include <catch2/catch_test_macros.hpp>

#include <gustave/units/stdStrict/lib/Math.hpp>
#include <gustave/units/stdStrict/lib/Real.hpp>
#include <gustave/units/stdStrict/lib/Unit.hpp>

namespace u = gustave::units::stdStrict::lib;

TEST_CASE("Math/") {
    struct Kilogram : u::BasicUnitIdentifier<"kg"> {};
    constexpr auto kg = u::makeUnitIdentifier<Kilogram>();

    SECTION("sqrt(cReal auto)") {
        SECTION("// dim one") {
            constexpr u::cReal auto val = 4.0 * u::one;
            REQUIRE(u::sqrt(val) == 2.0 * u::one);
        }

        SECTION("// kg ^ 2") {
            constexpr u::cReal auto val = 16.0 * (kg * kg);
            REQUIRE(u::sqrt(val) == 4.0 * kg);
        }
    }
}
