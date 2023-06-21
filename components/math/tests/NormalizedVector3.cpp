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
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "TestConfig.hpp"

#include <gustave/math/NormalizedVector3.hpp>

template<G::Cfg::cUnitOf<rt> auto unit>
using Vector3 = G::Math::Vector3<rt, unit>;

using NormalizedVector3 = G::Math::NormalizedVector3<rt>;

TEST_CASE("NormalizedVector3") {
    SECTION("::NormalizedVector3(Real<one>, Real<one>, Real<one>)") {
        NormalizedVector3 v{ 2.0, -1.0, -2.0 };
        CHECK_THAT(v.x().value(), Catch::Matchers::WithinRel(2.0 / 3.0, epsilon));
        CHECK_THAT(v.y().value(), Catch::Matchers::WithinRel(-1.0 / 3.0, epsilon));
        CHECK_THAT(v.z().value(), Catch::Matchers::WithinRel(-2.0 / 3.0, epsilon));
    }

    SECTION("::NormalizedVector3(Vector3<*> const&)") {
        NormalizedVector3 v{ Vector3<u.mass>{ 4.0, -2.0, 4.0, u.mass} };
        CHECK_THAT(v.x().value(), Catch::Matchers::WithinRel(4.0 / 6.0, epsilon));
        CHECK_THAT(v.y().value(), Catch::Matchers::WithinRel(-2.0 / 6.0, epsilon));
        CHECK_THAT(v.z().value(), Catch::Matchers::WithinRel(4.0 / 6.0, epsilon));
    }

    SECTION("") {
        const NormalizedVector3 v{ 2.0, -1.0, -2.0 };

        SECTION("::value()") {
            CHECK_THAT(v.value().x().value(), Catch::Matchers::WithinRel(2.0 / 3.0, epsilon));
        }

        SECTION("::x()") {
            CHECK_THAT(v.x().value(), Catch::Matchers::WithinRel(2.0 / 3.0, epsilon));
        }

        SECTION("::y()") {
            CHECK_THAT(v.y().value(), Catch::Matchers::WithinRel(-1.0 / 3.0, epsilon));
        }

        SECTION("::z()") {
            CHECK_THAT(v.z().value(), Catch::Matchers::WithinRel(-2.0 / 3.0, epsilon));
        }

        SECTION("::operator-()") {
            NormalizedVector3 opposed = -v;
            CHECK_THAT(opposed.x().value(), Catch::Matchers::WithinRel(-2.0 / 3.0, epsilon));
            CHECK_THAT(opposed.y().value(), Catch::Matchers::WithinRel( 1.0 / 3.0, epsilon));
            CHECK_THAT(opposed.z().value(), Catch::Matchers::WithinRel( 2.0 / 3.0, epsilon));
        }

        SECTION("operator*(Real, cNormalizedVector3 auto const&)") {
            const Vector3<u.force> res = (-3.0 * u.force) * v;
            CHECK_THAT(res.x().value(), Catch::Matchers::WithinRel(-2.0, epsilon));
            CHECK_THAT(res.y().value(), Catch::Matchers::WithinRel( 1.0, epsilon));
            CHECK_THAT(res.z().value(), Catch::Matchers::WithinRel( 2.0, epsilon));
        }

        SECTION("operator*(std::floating_point, cNormalizedVector3 auto const&)") {
            const Vector3<u.one> res = 3.0 * v;
            CHECK_THAT(res.x().value(), Catch::Matchers::WithinRel( 2.0, epsilon));
            CHECK_THAT(res.y().value(), Catch::Matchers::WithinRel(-1.0, epsilon));
            CHECK_THAT(res.z().value(), Catch::Matchers::WithinRel(-2.0, epsilon));
        }
    }
}
