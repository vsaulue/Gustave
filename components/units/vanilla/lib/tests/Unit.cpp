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

#include <sstream>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <gustave/units/lib/Unit.hpp>

namespace U = Gustave::Units::Lib;

template<U::cUnitIdentifier Id>
constexpr U::Unit<"", Id{}> makeUnit(Id) {
    return {};
}

TEST_CASE("Unit") {
    struct Kilogram : U::BasicUnitIdentifier<"kg"> {};
    constexpr auto kg = U::makeUnitIdentifier<Kilogram>();

    struct Metre : U::BasicUnitIdentifier<"m"> {};
    constexpr auto m = U::makeUnitIdentifier<Metre>();

    struct Second : U::BasicUnitIdentifier<"s"> {};
    constexpr auto s = U::makeUnitIdentifier<Second>();

    constexpr U::Unit<"N", kg * m / s.pow(U::Exponent<2>{})> newton{};

    constexpr U::Unit<"rad", U::UnitIdentifier<>{}> rad{};

    SECTION("::Unit(UnitIdentifier) // template argument deduction") {
        constexpr U::cUnit auto kgUnit = U::Unit(kg);
        CHECK(kgUnit.unitId() == kg);
        CHECK(kgUnit.symbol() == std::string_view("kg"));
    }

    SECTION("::isAssignableFrom()") {
        SECTION("// true") {
            constexpr U::cUnitIdentifier auto otherId = m / s * kg / s;
            constexpr U::Unit otherUnit = {otherId};
            CHECK(newton.isAssignableFrom(otherUnit));
            CHECK(newton.isAssignableFrom(otherId));
        }

        SECTION("// false") {
            constexpr U::cUnitIdentifier auto otherId = m / s * kg;
            constexpr U::Unit otherUnit = {otherId};
            CHECK(!newton.isAssignableFrom(otherUnit));
            CHECK(!newton.isAssignableFrom(otherId));
        }
    }

    SECTION("::isOne()") {
        SECTION("// true") {
            CHECK(rad.isOne());
            CHECK(U::one.isOne());
        }

        SECTION("// false") {
            CHECK_FALSE(newton.isOne());
        }
    }

    SECTION("::isTrivialOne()") {
        SECTION("// true") {
            CHECK(U::one.isTrivialOne());
        }

        SECTION("// false") {
            CHECK_FALSE(rad.isTrivialOne());
        }
    }

    SECTION("::inverse()") {
        constexpr U::Unit inv = newton.inverse();
        CHECK(inv.isAssignableFrom(s.pow(U::Exponent<2>{}) / kg / m));
    }

    SECTION("::pow(cExponent auto)") {
        constexpr U::Unit sqrM = { m * m };
        constexpr U::Unit res = sqrM.pow(U::Exponent<-1, 2>{});
        CHECK(res.isAssignableFrom(m.inverse()));
    }

    SECTION("operator==(cUnit auto, cUnit auto)") {
        SECTION("// true") {
            CHECK(newton == newton);
        }

        SECTION("// false: non-assignable") {
            CHECK(newton != U::Unit(s));
        }

        SECTION("// false: assignable, different symbol") {
            constexpr U::Unit newtonLike = { kg * m / s.pow(U::Exponent<2>{}) };
            CHECK(newton != newtonLike);
            CHECK(newton.isAssignableFrom(newtonLike));
        }
    }

    SECTION("operator*(cUnit auto, cUnit auto)") {
        constexpr auto res = newton * U::Unit(s);
        CHECK(res.isAssignableFrom(kg * m / s));
    }

    SECTION("operator/(cUnit auto, cUnit auto)") {
        constexpr auto res = newton / U::Unit(kg);
        CHECK(res.isAssignableFrom(m / s.pow(U::Exponent<2>{})));
    }

    SECTION("operator<<(std::ostream&, cUnit auto)") {
        std::stringstream stream;
        stream << newton;
        CHECK(stream.view() == std::string_view("N"));
    }
}
