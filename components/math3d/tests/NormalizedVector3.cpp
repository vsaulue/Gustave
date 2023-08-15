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

#include <gustave/testing/Matchers.hpp>
#include <gustave/math3d/NormalizedVector3.hpp>
#include <gustave/math3d/Vector3.hpp>

using NormalizedVector3 = G::Math3d::NormalizedVector3<rt>;

namespace M = Gustave::Testing::Matchers;

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

        SECTION(".dot()") {
            SECTION("// with Vector3") {
                Vector3<u.length> const v2{ 3.f, 0.f, 6.f, u.length };
                auto const matcher = M::WithinRel(-2.f * u.length, epsilon);
                CHECK_THAT(v.dot(v2), matcher);
                CHECK_THAT(v2.dot(v), matcher);
            }

            SECTION("// with NormalizedVector3") {
                NormalizedVector3 const v2{ 0.f,-1.f,0.f };
                auto const matcher = M::WithinRel(1.f / 3.f * u.one, epsilon);
                CHECK_THAT(v.dot(v2), matcher);
                CHECK_THAT(v2.dot(v), matcher);
            }
        }

        SECTION("::operator-()") {
            NormalizedVector3 opposed = -v;
            CHECK_THAT(opposed.x().value(), Catch::Matchers::WithinRel(-2.0 / 3.0, epsilon));
            CHECK_THAT(opposed.y().value(), Catch::Matchers::WithinRel( 1.0 / 3.0, epsilon));
            CHECK_THAT(opposed.z().value(), Catch::Matchers::WithinRel( 2.0 / 3.0, epsilon));
        }

        SECTION("::operator==(cVector3ConstArg auto const&)") {
            NormalizedVector3 opposed = -v;
            CHECK(v == v);
            CHECK(v != opposed);
            CHECK(v == v.value());
            CHECK(v != opposed.value());
        }

        SECTION("::operator+(cVector3ConstArg auto const&)") {
            NormalizedVector3 const v2{ -1.f, 0.f, 0.f };
            auto const matcher = M::WithinRel(vector3(-1.f, -1.f, -2.f, u.one) / 3.f, epsilon);
            CHECK_THAT(v + v2, matcher);
            CHECK_THAT(v + v2.value(), matcher);
            CHECK_THAT(v.value() + v2, matcher);
        }

        SECTION("::operator-(cVector3ConstArg auto const&)") {
            NormalizedVector3 const v2{ 0.f, -1.f, 0.f };
            auto const matcher = M::WithinRel(vector3(2.f, 2.f, -2.f, u.one) / 3.f, epsilon);
            CHECK_THAT(v - v2, matcher);
            CHECK_THAT(v - v2.value(), matcher);
            CHECK_THAT(v.value() - v2, matcher);
        }

        SECTION("// Multiplication: Real & NormalizedVector3 (both ways).") {
            Real<u.force> r = (-3.f * u.force);
            auto const matcher = M::WithinRel(vector3(-2.f, 1.f, 2.f, u.force), epsilon);
            CHECK_THAT(r * v, matcher);
            CHECK_THAT(v * r, matcher);
        }

        SECTION("// Multiplication: float & NormalizedVector3 (both ways).") {
            auto const matcher = M::WithinRel(vector3(2.f, -1.f, -2.f, u.one), epsilon);
            CHECK_THAT(v * 3.f, matcher);
            CHECK_THAT(3.f * v, matcher);
        }

        SECTION("// Division: NormalizedVector3 & float") {
            CHECK_THAT(v / (1.f / 6.f), M::WithinRel(vector3(4.f, -2.f, -4.f, u.one), epsilon));
        }

        SECTION("// Division: NormalizedVector3 & float") {
            Real<u.one / u.area> const r = -1.f / (12.f * u.area);
            CHECK_THAT(v / r, M::WithinRel(vector3(-8.f, 4.f, 8.f, u.area), epsilon));
        }

        SECTION("// convertible to Vector3") {
            Vector3<u.one> vec{ v };
            CHECK(vec == v);
        }

        SECTION("// assignable to Vector3") {
            auto vec = Vector3<u.one>::zero();
            vec = v;
            CHECK(vec == v);
        }
    }
}
