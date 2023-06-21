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

#include <gustave/math/Vector3.hpp>

#include "TestConfig.hpp"

template<G::Cfg::cUnitOf<rt> auto unit>
using Vector3 = G::Math::Vector3<rt, unit>;

TEST_CASE("Vector3") {
    constexpr auto acc = u.acceleration;
    constexpr auto kg = u.mass;
    constexpr auto m2 = u.area;
    constexpr auto newton = u.force;
    constexpr auto pa = u.pressure;

    SECTION("::Vector3(Real, Real, Real)") {
        constexpr Vector3<kg> v{ 1.0 * kg, 2.0 * kg, 3.0 * kg};
        CHECK(v.values[2] == 3.0 * kg);
    }

    SECTION("::Vector3(double, double, double, unit)") {
        constexpr Vector3<kg> v{ 1.0, 2.0, 3.0, kg };
        CHECK(v.values[2] == 3.0 * kg);
    }

    SECTION("::Vector3(cVector3 auto const& other)") {
        constexpr Vector3<newton / m2> src{ 3.0, 2.0, 1.0, newton / m2 };
        constexpr Vector3<pa> dst{ src };
        CHECK(dst.values[2] == 1.0 * pa);
    }
    
    SECTION("::zero()") {
        constexpr auto vZero = Vector3<kg>::zero();
        constexpr auto cZero = Real<kg>::zero();
        CHECK(vZero == Vector3<kg>{cZero, cZero, cZero});
    }

    SECTION("::operator=(cVector3 auto const&)") {
        constexpr Vector3<newton / m2> src{ 3.0, 2.0, 1.0, newton / m2 };
        Vector3<pa> dst{ 0.0, 0.0, 0.0, pa };
        const auto& res = (dst = src);
        CHECK(&res == &dst);
        CHECK(dst.values[0] == 3.0 * pa);
        CHECK(dst.values[1] == 2.0 * pa);
        CHECK(dst.values[2] == 1.0 * pa);
    }

    SECTION(".norm()") {
        Vector3<kg> vec{ 1.0, 2.0, -2.0, kg };
        Real<kg> norm = vec.norm();
        CHECK_THAT(norm.value(), Catch::Matchers::WithinRel(3.0, epsilon));
    }

    SECTION(".dot()") {
        constexpr Vector3<u.acceleration> lhs{ 1.0, 2.0, 3.0, acc };
        constexpr Vector3<kg> rhs{ 1.0, -2.0, 4.0, kg };
        constexpr Real<u.force> res = lhs.dot(rhs);
        CHECK(res == 9.0 * newton);
    }

    SECTION("::operator-()") {
        const Vector3<newton> v{ 3.0, 2.0, 1.0, newton };
        const auto res = -v;
        CHECK(res.unit() == newton);
        CHECK(res == Vector3<newton>(- 3.0, -2.0, -1.0, newton));
    }

    SECTION("") {
        Vector3<kg> lhs{ 1.0, 2.0, 3.0, kg };
        const Vector3<kg> rhs{ 2.0, 0.0, 5.0, kg };

        SECTION("::operator+=(Vector3<*,*> const&)") {
            Vector3<kg>& res = (lhs += rhs);
            CHECK(lhs == Vector3<kg>{ 3.0, 2.0, 8.0, kg });
            CHECK(&lhs == &res);
        }

        SECTION("::operator-=(Vector3<*,*> const&)") {
            Vector3<kg>& res = (lhs -= rhs);
            CHECK(lhs == Vector3<kg>{-1.0, 2.0, -2.0, kg });
            CHECK(&lhs == &res);
        }
    }

    SECTION("") {
        const Vector3<pa> lhs{ 1.0, 2.0, 3.0, pa };
        const Vector3<newton / m2> rhs{ 5.0, 7.0, -2.0, pa };

        SECTION("::operator+(Vector3<*,*> const&)") {
            const auto res = lhs + rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res.x() == 6.0 * pa);
            CHECK(res.y() == 9.0 * pa);
            CHECK(res.z() == 1.0 * pa);
        }

        SECTION("::operator-(Vector3<*,*> const&") {
            const auto res = lhs - rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res.x() == -4.0 * pa);
            CHECK(res.y() == -5.0 * pa);
            CHECK(res.z() == 5.0 * pa);
        }
    }

    SECTION("") {
        Vector3<kg> v{ 1.0, 2.0, 3.0, kg };
        const auto& cv = v;

        SECTION("::x()") {
            v.x() = 4.0 * kg;
            CHECK(cv.x() == 4.0 * kg);
        }

        SECTION("::y()") {
            v.y() = 4.0 * kg;
            CHECK(cv.y() == 4.0 * kg);
        }

        SECTION("::z()") {
            v.z() = 4.0 * kg;
            CHECK(cv.z() == 4.0 * kg);
        }
    }

    SECTION("") {
        constexpr Vector3<kg> vec{ 2.0, -4.0, 3.0, kg };

        SECTION("operator*(std::floating_point auto, cVector3 auto const&)") {
            constexpr G::Math::cVector3 auto res = 2.0 * vec;
            CHECK(res == Vector3<kg>{ 4.0, -8.0, 6.0, kg });
        }

        SECTION("operator*(cReal auto, cVector3 auto const&)") {
            constexpr G::Math::cVector3 auto res = (0.5 * kg) * vec;
            CHECK(res == Vector3<kg*kg>{ 1.0, -2.0, 1.5, kg*kg });
        }

        SECTION("operator/(cVector3 auto const&, cReal auto)") {
            constexpr G::Math::cVector3 auto res = vec / -2.0;
            CHECK(res == Vector3<kg>{ -1.0, 2.0, -1.5, kg });
        }

        SECTION("operator/(cVector3 auto const&, std::floating_point auto)") {
            constexpr G::Math::cVector3 auto res = vec / (0.5 * kg);
            CHECK(res == Vector3<u.one>{ 4.0, -8.0, 6.0, u.one });
        }
    }

    SECTION("operator==(cVector3 auto const&, cVector3 auto const&)") {
        const Vector3<u.pressure> lhs{ 2.0, -3.0, 4.0, pa };

        SECTION("// false") {
            CHECK(lhs != Vector3<u.pressure>{ 2.0, -3.0, 0.0, pa });
        }

        SECTION("//true") {
            CHECK(lhs == Vector3<u.pressure>{ 2.0, -3.0, 4.0, pa });
        }
    }

    SECTION("operator<<(std::ostream&, cVector3 auto const&)") {
        Vector3<u.pressure> vec{1.0, -2.0, 3.0, pa};
        std::stringstream stream;
        stream << vec;
        std::stringstream expected;
        expected << "{" << 1.0 << ", " << -2.0 << ", " << 3.0 << "}Pa";
        CHECK(stream.view() == expected.view());
    }
}
