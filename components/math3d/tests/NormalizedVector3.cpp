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
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <gustave/testing/Matchers.hpp>
#include <gustave/math3d/NormalizedVector3.hpp>
#include <gustave/math3d/Vector3.hpp>
#include <TestHelpers.hpp>

using NormalizedVector3 = gustave::math3d::NormalizedVector3<rt, float>;

namespace matchers = gustave::testing::matchers;

TEST_CASE("NormalizedVector3") {
    constexpr auto minNormalized = std::numeric_limits<float>::min();
    auto const lowNormVec = vector3(0.5f * std::sqrt(minNormalized), 0.f, 0.f, u.one);

    SECTION("::NormalizedVector3(Real<one>, Real<one>, Real<one>)") {
        SECTION("// valid") {
            auto const v = NormalizedVector3{ 2.f, -1.f, -2.f };
            CHECK_THAT(v.value(), matchers::WithinRel(vector3(2.f, -1.f, -2.f, u.one) / 3.f, epsilon));
        }

        SECTION("// invalid (norm too low)") {
            CHECK_THROWS_AS(NormalizedVector3( lowNormVec.x(), lowNormVec.y(), lowNormVec.z()), std::domain_error);
        }
    }

    SECTION("::NormalizedVector3(Vector3<*> const&)") {
        SECTION("// valid") {
            auto const v = NormalizedVector3{ vector3(4.f, -2.f, 4.f, u.mass) };
            CHECK_THAT(v.value(), matchers::WithinRel(vector3(2.f, -1.f, 2.f, u.one) / 3.f, epsilon));
        }

        SECTION("// invalid (norm too low)") {
            CHECK_THROWS_AS(NormalizedVector3( lowNormVec ), std::domain_error);
        }
    }

    SECTION("") {
        NormalizedVector3 const v{ 2.f, -1.f, -2.f };

        SECTION("::value()") {
            CHECK_THAT(v.value().x(), matchers::WithinRel((2.f / 3.f) * u.one, epsilon));
        }

        SECTION("::x()") {
            CHECK_THAT(v.x(), matchers::WithinRel((2.f / 3.f) * u.one, epsilon));
        }

        SECTION("::y()") {
            CHECK_THAT(v.y(), matchers::WithinRel((-1.f / 3.f) * u.one, epsilon));
        }

        SECTION("::z()") {
            CHECK_THAT(v.z(), matchers::WithinRel((- 2.f / 3.f) * u.one, epsilon));
        }

        SECTION(".dot()") {
            SECTION("// with Vector3") {
                auto const v2 = vector3(3.f, 0.f, 6.f, u.length);
                auto const matcher = matchers::WithinRel(-2.f * u.length, epsilon);
                CHECK_THAT(v.dot(v2), matcher);
                CHECK_THAT(v2.dot(v), matcher);
            }

            SECTION("// with NormalizedVector3") {
                NormalizedVector3 const v2{ 0.f,-1.f,0.f };
                auto const matcher = matchers::WithinRel(1.f / 3.f * u.one, epsilon);
                CHECK_THAT(v.dot(v2), matcher);
                CHECK_THAT(v2.dot(v), matcher);
            }
        }

        SECTION("::operator-()") {
            NormalizedVector3 const opposed = -v;
            CHECK_THAT(opposed.value(), matchers::WithinRel(vector3(-2.f, 1.f, 2.f, u.one) / 3.f, epsilon));
        }

        SECTION("::operator==(cVector3ConstArg auto const&)") {
            NormalizedVector3 const opposed = -v;
            CHECK(v == v);
            CHECK(v != opposed);
            CHECK(v == v.value());
            CHECK(v != opposed.value());
        }

        SECTION("::operator+(cVector3ConstArg auto const&)") {
            NormalizedVector3 const v2{ -1.f, 0.f, 0.f };
            auto const matcher = matchers::WithinRel(vector3(-1.f, -1.f, -2.f, u.one) / 3.f, epsilon);
            CHECK_THAT(v + v2, matcher);
            CHECK_THAT(v + v2.value(), matcher);
            CHECK_THAT(v.value() + v2, matcher);
        }

        SECTION("::operator-(cVector3ConstArg auto const&)") {
            NormalizedVector3 const v2{ 0.f, -1.f, 0.f };
            auto const matcher = matchers::WithinRel(vector3(2.f, 2.f, -2.f, u.one) / 3.f, epsilon);
            CHECK_THAT(v - v2, matcher);
            CHECK_THAT(v - v2.value(), matcher);
            CHECK_THAT(v.value() - v2, matcher);
        }

        SECTION("// Multiplication: Real & NormalizedVector3 (both ways).") {
            auto r = -3.f * u.force;
            auto const matcher = matchers::WithinRel(vector3(-2.f, 1.f, 2.f, u.force), epsilon);
            CHECK_THAT(r * v, matcher);
            CHECK_THAT(v * r, matcher);
        }

        SECTION("// Multiplication: float & NormalizedVector3 (both ways).") {
            auto const matcher = matchers::WithinRel(vector3(2.f, -1.f, -2.f, u.one), epsilon);
            CHECK_THAT(v * 3.f, matcher);
            CHECK_THAT(3.f * v, matcher);
        }

        SECTION("// Division: NormalizedVector3 & float") {
            CHECK_THAT(v / (1.f / 6.f), matchers::WithinRel(vector3(4.f, -2.f, -4.f, u.one), epsilon));
        }

        SECTION("// Division: NormalizedVector3 & float") {
            auto const r = -1.f / (12.f * u.area);
            CHECK_THAT(v / r, matchers::WithinRel(vector3(-8.f, 4.f, 8.f, u.area), epsilon));
        }

        SECTION("// convertible to Vector3") {
            Vector3<u.one, double> vec{ v };
            CHECK(vec == v);
        }

        SECTION("// assignable to Vector3") {
            auto vec = Vector3<u.one, double>::zero();
            vec = v;
            CHECK(vec == v);
        }
    }
}
