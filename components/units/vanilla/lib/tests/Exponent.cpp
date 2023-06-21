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

#include <catch2/catch_test_macros.hpp>

#include <gustave/units/lib/Exponent.hpp>

namespace U = Gustave::Units::Lib;

TEST_CASE("Exponent") {
    SECTION("::operator-()") {
        constexpr auto exp = U::Exponent<3, 2>();
        constexpr U::cExponent auto res = -exp;
        REQUIRE(res.num() == -3);
        REQUIRE(res.den() == 2);
    }

    SECTION("operator+(cExponent auto,cExponent auto)") {
        constexpr auto lhs = U::Exponent<7, 4>();
        constexpr auto rhs = U::Exponent<-1, 4>();
        constexpr U::cExponent auto res = lhs + rhs;
        REQUIRE(res.num() == 3);
        REQUIRE(res.den() == 2);
    }

    SECTION("operator-(cExponent auto, cExponent auto)") {
        constexpr auto lhs = U::Exponent<1,4>();
        constexpr auto rhs = U::Exponent<1,3>();
        constexpr U::cExponent auto res = lhs - rhs;
        REQUIRE(res.num() == -1);
        REQUIRE(res.den() == 12);
    }

    SECTION("operator*(cExponent auto, cExponent auto)") {
        constexpr auto lhs = U::Exponent<5, 6>();
        constexpr auto rhs = U::Exponent<-2,5>();
        constexpr U::cExponent auto res = lhs * rhs;
        REQUIRE(res.num() == -1);
        REQUIRE(res.den() == 3);
    }

    SECTION("operator==(cExponent auto, cExponent auto)") {
        constexpr auto lhs = U::Exponent<5, 6>{};

        SECTION("// true") {
            constexpr auto rhs = U::Exponent<5, 6>{};
            REQUIRE(lhs == rhs);
        }

        SECTION("// false") {
            constexpr auto rhs = U::Exponent<-5, 6>{};
            REQUIRE(lhs != rhs);
        }
    }

    SECTION("// zero simplification") {
        constexpr auto val = U::Exponent<1, 4>{};
        constexpr U::cExponent auto zero = val - val;
        REQUIRE(zero.num() == 0);
        REQUIRE(zero.den() == 1);
    }

    SECTION("::isZero()") {
        SECTION("// true") {
            constexpr auto zero = U::Exponent<0, 1>{};
            REQUIRE(zero.isZero());
        }

        SECTION("// false") {
            constexpr auto notZero = U::Exponent<-1, 1>{};
            REQUIRE(!notZero.isZero());
        }
    }

    SECTION("::toString()") {
        SECTION("// 0") {
            constexpr U::cExponent auto zero = U::Exponent<0, 1>{};
            REQUIRE(zero.toString().view() == "⁰");
        }

        SECTION("// -20") {
            constexpr U::cExponent auto val = U::Exponent<-20, 1>{};
            REQUIRE(val.toString().view() == "⁻²⁰");
        }

        SECTION("// 3/2") {
            constexpr U::cExponent auto val = U::Exponent<3, 2>{};
            REQUIRE(val.toString().view() == "³⁄²");
        }
    }
}
