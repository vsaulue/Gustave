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

#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <gustave/math3d/Vector3.hpp>
#include <gustave/testing/Matchers.hpp>
#include <TestHelpers.hpp>

namespace M = Gustave::Testing::Matchers;

TEST_CASE("Vector3") {
    constexpr auto acc = u.acceleration;
    constexpr auto kg = u.mass;
    constexpr auto m2 = u.area;
    constexpr auto newton = u.force;
    constexpr auto pa = u.pressure;

    SECTION("::Vector3(Real, Real, Real)") {
        constexpr Vector3<kg> v{ 1.f * kg, 2.f * kg, 3.f * kg};
        CHECK(v.coords()[0] == 1.f * kg);
        CHECK(v.coords()[1] == 2.f * kg);
        CHECK(v.coords()[2] == 3.f * kg);
    }

    SECTION("::Vector3(Rep, Rep, rep, unit)") {
        constexpr Vector3<kg> v{ 1.f, 2.f, 3.f, kg };
        CHECK(v.coords()[2] == 3.0 * kg);
    }

    SECTION("::Vector3(cVector3 auto const& other)") {
        constexpr auto src = vector3(3.f, 2.f, 1.f, newton / m2);
        constexpr Vector3<pa> dst{ src };
        CHECK(dst.coords()[2] == 1.f * pa);
    }
    
    SECTION("::zero()") {
        constexpr auto vZero = Vector3<kg>::zero();
        constexpr auto cZero = Real<kg>::zero();
        CHECK(vZero == Vector3<kg>{cZero, cZero, cZero});
    }

    SECTION("::operator=(cVector3 auto const&)") {
        constexpr auto src = vector3(3.f, 2.f, 1.f, newton / m2);
        Vector3<pa> dst{ 0.f, 0.f, 0.f, pa };
        const auto& res = (dst = src);
        CHECK(&res == &dst);
        CHECK(dst == vector3(3.f, 2.f, 1.f, pa));
    }

    SECTION(".norm()") {
        auto const vec = vector3(1.f, 2.f, -2.f, kg);
        Real<kg> norm = vec.norm();
        CHECK_THAT(norm, M::WithinRel(3.f * kg, epsilon));
    }

    SECTION(".dot()") {
        constexpr auto lhs = vector3(1.f, 2.f, 3.f, acc);
        constexpr auto rhs = vector3(1.f, -2.f, 4.f, kg);
        constexpr Real<u.force> res = lhs.dot(rhs);
        CHECK(res == 9.f * newton);
    }

    SECTION("::operator-()") {
        constexpr auto v = vector3(3.f, 2.f, 1.f, newton);
        constexpr auto res = -v;
        CHECK(res.unit() == newton);
        CHECK(res == vector3(-3.f, -2.f, -1.f, newton));
    }

    SECTION("") {
        auto lhs = vector3(1.f, 2.f, 3.f, kg);
        auto const rhs = vector3(2.f, 0.f, 5.f, kg);

        SECTION("::operator+=(cVector3ConstArg auto const&)") {
            Vector3<kg>& res = (lhs += rhs);
            CHECK(lhs == vector3(3.f, 2.f, 8.f, kg));
            CHECK(&lhs == &res);
        }

        SECTION("::operator-=(cVector3ConstArg auto const&)") {
            Vector3<kg>& res = (lhs -= rhs);
            CHECK(lhs == vector3(-1.f, 2.f, -2.f, kg));
            CHECK(&lhs == &res);
        }
    }

    SECTION("") {
        auto const lhs = vector3(1.f, 2.f, 3.f, pa);
        auto const rhs = vector3(5.f, 7.f, -2.f, pa);

        SECTION("::operator+(cVector3ConstArg auto const&)") {
            const auto res = lhs + rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == vector3(6.f, 9.f, 1.f, pa));
        }

        SECTION("::operator-(cVector3ConstArg auto const&") {
            const auto res = lhs - rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == vector3(-4.f, -5.f, 5.f, pa));
        }
    }

    SECTION("") {
        auto v = vector3(1.f, 2.f, 3.f, kg);
        const auto& cv = v;

        SECTION("::x()") {
            v.x() = 4.f * kg;
            CHECK(cv.x() == 4.f * kg);
        }

        SECTION("::y()") {
            v.y() = 4.f * kg;
            CHECK(cv.y() == 4.f * kg);
        }

        SECTION("::z()") {
            v.z() = 4.f * kg;
            CHECK(cv.z() == 4.f * kg);
        }
    }

    SECTION("") {
        constexpr auto vec = vector3(2.f, -4.f, 3.f, kg);

        SECTION("// Multiplication float * Vector3 (& reverse)") {
            constexpr auto expected = vector3(4.f, -8.f, 6.f, kg);
            CHECK(2.f * vec == expected);
            CHECK(vec * 2.f == expected);
        }

        SECTION("// Multiplication Real * Vector3 (& reverse)") {
            constexpr Real<kg> r = 0.5f * kg;
            constexpr auto expected = vector3(1.f, -2.f, 1.5f, kg * kg);
            CHECK(r * vec == expected);
            CHECK(vec * r == expected);
        }

        SECTION("// Division Vector3 / Real") {
            constexpr Real<kg> r = -2.f * kg;
            CHECK(vec / r == vector3(-1.f, 2.f, -1.5f, u.one));
        }

        SECTION("// Division Vector3 / float") {
            CHECK(vec / -1.f == vector3(-2.f, 4.f, -3.f, kg));
        }
    }

    SECTION("operator==(cVector3 auto const&, cVector3 auto const&)") {
        constexpr auto lhs = vector3(2.f, -3.f, 4.f, pa);

        SECTION("// false") {
            CHECK(lhs != vector3(2.f, -3.f, 0.f, pa));
        }

        SECTION("//true") {
            CHECK(lhs == vector3(2.f, -3.f, 4.f, pa));
        }
    }

    SECTION("operator<<(std::ostream&, Vector3 const&)") {
        auto const vec = vector3(1.f, -2.f, 3.f, pa);
        std::stringstream stream;
        stream << vec;
        std::stringstream expected;
        expected << "{" << 1.f << ", " << -2.f << ", " << 3.f << "}Pa";
        CHECK(stream.view() == expected.view());
    }
}
