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

#include <sstream>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include <gustave/units/stdStrict/lib/BasicUnitIdentifier.hpp>
#include <gustave/units/stdStrict/lib/Real.hpp>
#include <gustave/units/stdStrict/lib/Unit.hpp>
#include <gustave/units/stdStrict/lib/UnitIdentifier.hpp>

namespace u = gustave::units::stdStrict::lib;

static void testValue(u::cReal auto tested, std::floating_point auto expected) {
    using TestedRep = typename decltype(tested)::Rep;
    CHECK(std::is_same_v<TestedRep, decltype(expected)>);
    CHECK(tested.value() == expected);
}

TEST_CASE("Real") {
    struct Kilogram : u::BasicUnitIdentifier<"kg"> {};
    constexpr auto kg = u::makeUnitIdentifier<Kilogram>();

    struct Metre : u::BasicUnitIdentifier<"m"> {};
    constexpr auto m = u::makeUnitIdentifier<Metre>();

    struct Second : u::BasicUnitIdentifier<"s"> {};
    constexpr auto s = u::makeUnitIdentifier<Second>();

    constexpr u::Unit<"N", kg * m / s.pow(u::Exponent<2>{})> newton{};

    constexpr u::Unit<"NewtonLike", newton.unitId()> newtonLike{};

    // constructors

    SECTION("::Real(Rep, cUnit auto)") {
        using R = u::Real<newton, double>;
        const R val{ 5.f, newton };
        CHECK(val.unit().isAssignableFrom(newton));
        testValue(val, 5.0);
    }

    SECTION("::Real(Rep) // requires( unit_.isTrivialOne() )") {
        using R = u::Real<u::one, double>;
        const R val{ 7.0 };
        testValue(val, 7.0);
    }

    SECTION("operator*(std::floating_point auto, cUnit auto)") {
        auto val = 3.0f * newton;
        CHECK(val.unit() == newton);
        testValue(val, 3.0f);
    }

    SECTION("operator*(std::floating_point auto, cUnitIdentifier auto)") {
        auto val = -1.0 * m;
        CHECK(val.unit().isAssignableFrom(m));
        testValue(val, -1.0);
    }

    SECTION("operator/(std::floating_point auto, cUnit auto)") {
        auto val = 5.0 / newton;
        CHECK(val.unit().isAssignableFrom(newton.inverse()));
        testValue(val, 5.0);
    }

    SECTION("operator/(std::floating_point auto, cUnitIdentifier auto)") {
        auto val = -10.0f / s;
        CHECK(val.unit().isAssignableFrom(s.inverse()));
        testValue(val, -10.0f);
    }

    // addition

    SECTION("::operator+=(cReal auto)") {
        auto lhs = 3.0 * m;
        constexpr auto rhs = -1.0 * m;
        auto& ref = (lhs += rhs);
        testValue(lhs, 2.0);
        CHECK(&ref == &lhs);
    }

    SECTION("::operator+=(std::floating_point auto)") {
        auto lhs = -2.0 * u::one;
        auto& ref = (lhs += 3.0);
        testValue(lhs, 1.0);
        CHECK(&ref == &lhs);
    }

    SECTION("operator+(cReal auto, cReal auto)") {
        constexpr auto lhs = 4.0 * newton;
        constexpr auto rhs = 2.0 * newton;
        constexpr auto res = lhs + rhs;
        CHECK(res.unit() == lhs.unit());
        testValue(res, 6.0);
    }

    SECTION("operator+(cReal auto, std::floating_point auto)") {
        auto res = (3.0f * u::one) + 1.5;
        CHECK(res.unit() == u::one);
        testValue(res, 4.5);
    }

    SECTION("operator+(std::floating_point auto, cReal auto)") {
        auto res = -1.0f + (4.0 * u::one);
        CHECK(res.unit() == u::one);
        testValue(res, 3.0);
    }

    // substraction

    SECTION("::operator-=(cReal auto)") {
        auto lhs = 5.0 * m;
        auto rhs = -1.0 * m;
        auto& ref = (lhs -= rhs);
        testValue(lhs, 6.0);
        CHECK(&ref == &lhs);
    }

    SECTION("::operator-=(std::floating_point auto)") {
        auto lhs = 2.0 * u::one;
        auto& ref = (lhs -= 3.0f);
        testValue(lhs, -1.0);
        CHECK(&ref == &lhs);
    }

    SECTION("operator-(cReal auto, cReal auto)") {
        auto lhs = 2.0f * s;
        auto rhs = -1.0f * s;
        auto res = lhs - rhs;
        CHECK(res.unit() == lhs.unit());
        testValue(res, 3.0f);
    }

    SECTION("operator-(cReal auto, std::floating_point auto)") {
        auto res = (1.0 * u::one) - 4.0;
        CHECK(res.unit() == u::one);
        testValue(res, -3.0);
    }

    SECTION("operator-(cReal auto, std::floating_point auto)") {
        auto res = (-2.0 * u::one) - (-0.5f);
        CHECK(res.unit() == u::one);
        testValue(res, -1.5);
    }

    // multiplication

    SECTION("::operator*=(cReal auto)") {
        auto lhs = -2.0 * m;
        auto& ref = lhs *= (3.5 * u::one);
        testValue(lhs, -7.0);
        CHECK(&ref == &lhs);
    }

    SECTION("::operator*=(std::floating_point auto)") {
        auto lhs = -3.0 * s;
        auto& ref = lhs *= -0.5f;
        testValue(lhs, 1.5);
        CHECK(&ref == &lhs);
    }

    SECTION("operator*(cReal auto, cReal auto)") {
        auto lhs = 3.0f * m;
        auto rhs = 0.5 / s;
        auto res = lhs * rhs;
        CHECK(res.unit().isAssignableFrom(m / s));
        testValue(res, 1.5);
    }

    SECTION("operator*(cReal auto, std::floating_point auto)") {
        auto lhs = 3.0f * newton;
        auto res = lhs * -2.0f;
        CHECK(res.unit() == newton);
        testValue(res, -6.0f);
    }

    SECTION("operator*(std::floating_point auto, cReal auto") {
        auto res = 2.0 * (5.0f * newton);
        CHECK(res.unit() == newton);
        testValue(res, 10.0);
    }

    // division

    SECTION("::operator/=(cReal auto)") {
        auto lhs = 6.0f * s;
        auto& ref = lhs /= (-2.0f * u::one);
        testValue(lhs, -3.0f);
        CHECK(&ref == &lhs);
    }

    SECTION("::operator/=(std::floating_point auto)") {
        auto lhs = 4.0 * m;
        auto& ref = lhs /= 0.5;
        testValue(lhs, 8.0);
        CHECK(&ref == &lhs);
    }

    SECTION("operator/(cReal auto, cReal auto)") {
        auto res = (3.0 * m) / (0.5f * s);
        CHECK(res.unit().isAssignableFrom(m / s));
        testValue(res, 6.0);
    }

    SECTION("operator/(cReal auto, std::floating_point auto)") {
        auto res = (3.0f * newton) / 6.0f;
        CHECK(res.unit() == newton);
        testValue(res, 0.5f);
    }

    SECTION("operator/(std::floating_point auto, cReal auto)") {
        auto res = 6.0f / (3.0 / s);
        CHECK(res.unit().isAssignableFrom(s));
        testValue(res, 2.0);
    }

    // equality
    SECTION("operator==(cReal auto, cReal auto)") {
        SECTION("// true") {
            const auto lhs = 3.0 * newton;
            const auto rhs = 3.0f * newtonLike;
            CHECK(lhs == rhs);
        }

        SECTION("// false") {
            const auto lhs = 3.0f * kg;
            const auto rhs = 1.0f * kg;
            CHECK(lhs != rhs);
        }
    }

    SECTION("operator==(cReal auto, std::floating_point auto)") {
        SECTION("// true") {
            const auto lhs = -3.0 * u::one;
            const auto rhs = -3.0f;
            CHECK(lhs == rhs);
        }

        SECTION("// false") {
            const auto lhs = 2.0 * u::one;
            const auto rhs = 0.0;
            CHECK(lhs != rhs);
        }
    }

    SECTION("operator==(std::floating_point auto, cReal auto)") {
        SECTION("// true") {
            const auto lhs = 0.5f;
            const auto rhs = 0.5f * u::one;
            CHECK(lhs == rhs);
        }

        SECTION("// false") {
            const auto lhs = 1.5f;
            const auto rhs = 0.0 * u::one;
            CHECK(lhs != rhs);
        }
    }

    // comparison

    SECTION("operator<=>(cReal auto, cReal auto)") {
        const auto lhs = 0.5 * newton;
        const auto rhs = 0.75f * newtonLike;
        CHECK(lhs < rhs);
    }

    SECTION("operator<=>(cReal auto, std::floating_point)") {
        const auto lhs = 2.0 * u::one;
        const auto rhs = -1.0f;
        CHECK(lhs > rhs);
    }

    SECTION("operator<=>(std::floating_point, cReal auto)") {
        const auto lhs = 3.0;
        const auto rhs = 5.0 * u::one;
        CHECK(lhs < rhs);
    }

    // other

    SECTION("::zero()") {
        constexpr auto val = u::Real<newton, double>::zero();
        CHECK(val.unit().isAssignableFrom(newton));
        testValue(val, 0.0);
    }

    SECTION("::operator-()") {
        constexpr auto val = 2.5 * (m / s);
        constexpr auto res = -val;
        CHECK(res.unit() == val.unit());
        testValue(res, -2.5);
    }

    SECTION("::operator=(cReal auto)") {
        auto val = 2.5 * newton;
        auto& ref = (val = 3.5f * newtonLike);
        testValue(val, 3.5);
        CHECK(&ref == &val);
    }

    SECTION("::isCompatible(cUnit auto)") {
        CHECK(u::Real<newton, double>::isCompatible(newtonLike));
        CHECK_FALSE(u::Real<newton, double>::isCompatible(u::Unit{ kg }));
    }

    SECTION("operator<<(std::ostream&, cReal auto)") {
        constexpr auto val = -1.0 * newton;
        std::stringstream stream;
        stream << val;
        std::stringstream expected;
        expected << val.value() << " N";
        CHECK(stream.view() == expected.view());
    }
}
