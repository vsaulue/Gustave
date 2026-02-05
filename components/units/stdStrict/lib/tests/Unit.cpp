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

#include <sstream>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <gustave/units/stdStrict/lib/Unit.hpp>

namespace u = gustave::units::stdStrict::lib;

template<u::cUnitIdentifier Id>
constexpr u::Unit<"", Id{}> makeUnit(Id) {
    return {};
}

TEST_CASE("Unit") {
    struct Kilogram : u::BasicUnitIdentifier<"kg"> {};
    constexpr auto kg = u::makeUnitIdentifier<Kilogram>();

    struct Metre : u::BasicUnitIdentifier<"m"> {};
    constexpr auto m = u::makeUnitIdentifier<Metre>();

    struct Second : u::BasicUnitIdentifier<"s"> {};
    constexpr auto s = u::makeUnitIdentifier<Second>();

    constexpr u::Unit<"N", kg * m / s.pow(u::Exponent<2>{})> newton{};

    constexpr u::Unit<"rad", u::UnitIdentifier<>{}> rad{};

    SECTION("::Unit(UnitIdentifier) // template argument deduction") {
        constexpr u::cUnit auto kgUnit = u::Unit(kg);
        CHECK(kgUnit.unitId() == kg);
        CHECK(kgUnit.symbol() == std::string_view("kg"));
    }

    SECTION("::isAssignableFrom()") {
        SECTION("// true") {
            constexpr u::cUnitIdentifier auto otherId = m / s * kg / s;
            constexpr u::Unit otherUnit = {otherId};
            CHECK(newton.isAssignableFrom(otherUnit));
            CHECK(newton.isAssignableFrom(otherId));
        }

        SECTION("// false") {
            constexpr u::cUnitIdentifier auto otherId = m / s * kg;
            constexpr u::Unit otherUnit = {otherId};
            CHECK(!newton.isAssignableFrom(otherUnit));
            CHECK(!newton.isAssignableFrom(otherId));
        }
    }

    SECTION("::isOne()") {
        SECTION("// true") {
            CHECK(rad.isOne());
            CHECK(u::one.isOne());
        }

        SECTION("// false") {
            CHECK_FALSE(newton.isOne());
        }
    }

    SECTION("::isTrivialOne()") {
        SECTION("// true") {
            CHECK(u::one.isTrivialOne());
        }

        SECTION("// false") {
            CHECK_FALSE(rad.isTrivialOne());
        }
    }

    SECTION("::inverse()") {
        constexpr u::Unit inv = newton.inverse();
        CHECK(inv.isAssignableFrom(s.pow(u::Exponent<2>{}) / kg / m));
    }

    SECTION("::pow(cExponent auto)") {
        constexpr u::Unit sqrM = { m * m };
        constexpr u::Unit res = sqrM.pow(u::Exponent<-1, 2>{});
        CHECK(res.isAssignableFrom(m.inverse()));
    }

    SECTION("operator==(cUnit auto, cUnit auto)") {
        SECTION("// true") {
            CHECK(newton == newton);
        }

        SECTION("// false: non-assignable") {
            CHECK(newton != u::Unit(s));
        }

        SECTION("// false: assignable, different symbol") {
            constexpr u::Unit newtonLike = { kg * m / s.pow(u::Exponent<2>{}) };
            CHECK(newton != newtonLike);
            CHECK(newton.isAssignableFrom(newtonLike));
        }
    }

    SECTION("operator*(cUnit auto, cUnit auto)") {
        constexpr auto res = newton * u::Unit(s);
        CHECK(res.isAssignableFrom(kg * m / s));
    }

    SECTION("operator/(cUnit auto, cUnit auto)") {
        constexpr auto res = newton / u::Unit(kg);
        CHECK(res.isAssignableFrom(m / s.pow(u::Exponent<2>{})));
    }

    SECTION("operator<<(std::ostream&, cUnit auto)") {
        std::stringstream stream;
        stream << newton;
        CHECK(stream.view() == std::string_view("N"));
    }
}
