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

#include <gustave/units/lib/UnitIdentifier.hpp>

namespace U = Gustave::Units::Lib;

template<U::cBasicUnitIdentifier id, U::ExpNum num, U::ExpDen den = 1>
using Term = U::UnitTerm<id{}, U::Exponent<num, den>{}> ;

TEST_CASE("UnitIdentifier") {
    struct Kilogram : U::BasicUnitIdentifier<"kg"> {};
    constexpr auto kg = U::makeUnitIdentifier<Kilogram>();

    struct Metre : U::BasicUnitIdentifier<"m"> {};
    constexpr auto m = U::makeUnitIdentifier<Metre>();

    struct Second : U::BasicUnitIdentifier<"s"> {};
    constexpr auto s = U::makeUnitIdentifier<Second>();

    constexpr U::UnitIdentifier<> one{};

    SECTION("::inverse()") {
        constexpr U::UnitIdentifier<Term<Kilogram, -1, 2>{}, Term<Metre, 3>{}> arg{};
        constexpr U::UnitIdentifier<Term<Kilogram, 1, 2>{}, Term<Metre, -3>{} > expected{};
        REQUIRE(arg.inverse() == expected);
        REQUIRE(arg != expected);
    }

    SECTION("::isOne()") {
        SECTION("// true") {
            REQUIRE(one.isOne());
        }

        SECTION("// false") {
            constexpr U::cUnitIdentifier auto speed = m / s;
            REQUIRE(!speed.isOne());
        }
    }

    SECTION("::pow()") {
        SECTION("// one ^ exp") {
            constexpr U::Exponent<2, 1> exp{};
            REQUIRE(one.pow(exp) == one);
        }

        SECTION("// non-trivial ^ exp") {
            constexpr U::Exponent<-2, 3> exp{};
            constexpr U::UnitIdentifier<Term<Metre, 3>{}, Term<Second,-4>{}> val{};
            constexpr U::UnitIdentifier<Term<Metre, -2>{}, Term<Second, 8, 3>{}> expected{};
            REQUIRE(val.pow(exp) == expected);
        }

        SECTION("//non-trivial ^ 0") {
            constexpr U::Exponent<0, 1> exp{};
            constexpr U::UnitIdentifier<Term<Metre, 2>{}, Term<Second, -1>{}> val{};
            REQUIRE(val.pow(exp) == one);
        }
    }

    SECTION("::toString()") {
        SECTION(" // one") {
            REQUIRE(one.toString().view() == "");
        }

        SECTION(" // kg.m^-3") {
            constexpr U::UnitIdentifier<Term<Kilogram, 1>{}, Term<Metre, -3>{}> id{};
            REQUIRE(id.toString().view() == "kg.m⁻³");
        }
    }

    SECTION("operator==(cUnitIdentifier auto,cUnitIdentifier auto)") {
        constexpr U::UnitIdentifier<Term<Metre,2>{}> sqrMetre{};
        constexpr U::UnitIdentifier<Term<Second,-1>{}> hz{};
        REQUIRE(sqrMetre == sqrMetre);
        REQUIRE(sqrMetre != hz);
    }

    SECTION("operator*(cLikeUnitIdentifier auto, cLikeUnitIdentifier auto)") {
        constexpr U::cUnitIdentifier auto val = (kg * m) * (s * m);
        constexpr U::UnitIdentifier<Term<Kilogram, 1>{}, Term<Metre, 2>{}, Term<Second, 1>{}> exp{};
        REQUIRE(val == exp);
    }

    SECTION("operator-(cLikeUnitIdentifier auto, cLikeUnitIdentifier auto)") {
        constexpr U::cUnitIdentifier auto val = (kg / s * m / s) / kg;
        constexpr U::UnitIdentifier<Term<Metre, 1>{}, Term<Second, -2>{}> exp{};
        REQUIRE(val == exp);
    }
}
