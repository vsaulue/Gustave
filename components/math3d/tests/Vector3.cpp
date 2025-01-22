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

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <gustave/math3d/Vector3.hpp>
#include <gustave/testing/Matchers.hpp>
#include <TestHelpers.hpp>

namespace matchers = gustave::testing::matchers;

template<typename T, typename ExpectedRep>
concept cVectorHasRep = std::is_same_v<typename std::remove_cvref_t<T>::Coord::Rep, ExpectedRep>;

template<typename T, typename ExpectedRep>
concept cRealHasRep = std::is_same_v<typename std::remove_cvref_t<T>::Rep, ExpectedRep>;

TEST_CASE("Vector3") {
    constexpr auto acc = u.acceleration;
    constexpr auto kg = u.mass;
    constexpr auto m2 = u.area;
    constexpr auto newton = u.force;
    constexpr auto pa = u.pressure;

    SECTION("::Vector3(Real, Real, Real)") {
        constexpr Vector3<kg,float> v{ 1.f * kg, 2.f * kg, 3.f * kg};
        static_assert(cRealHasRep<decltype(v.coords()[0]), float>);
        CHECK(v.coords()[0] == 1.f * kg);
        CHECK(v.coords()[1] == 2.f * kg);
        CHECK(v.coords()[2] == 3.f * kg);
    }

    SECTION("::Vector3(Rep, Rep, rep, unit)") {
        constexpr Vector3<kg,float> v{ 1.f, 2.f, 3.f, kg };
        CHECK(v.coords()[2] == 3.f * kg);
    }

    SECTION("::Vector3(cVector3 auto const& other)") {
        SECTION("// float -> float") {
            constexpr auto src = vector3(3.f, 2.f, 1.f, newton / m2);
            constexpr Vector3<pa, float> dst{ src };
            CHECK(dst.coords()[2] == 1.f * pa);
        }

        SECTION("// float -> double") {
            constexpr auto src = vector3(3.f, 2.f, 1.f, newton / m2);
            constexpr Vector3<pa, double> dst{ src };
            CHECK(dst.coords()[2] == 1.f * pa);
        }
    }
    
    SECTION("::zero()") {
        cVectorHasRep<float> auto vZero = Vector3<kg, float>::zero();
        CHECK(vZero == vector3(0.f, 0.f, 0.f, kg));
    }

    SECTION("::operator=(cVector3 auto const&)") {
        SECTION("// float -> float") {
            constexpr auto src = vector3(3.f, 2.f, 1.f, newton / m2);
            Vector3<pa,float> dst{ 0.f, 0.f, 0.f, pa };
            const auto& res = (dst = src);
            CHECK(&res == &dst);
            CHECK(dst == vector3(3.f, 2.f, 1.f, pa));
        }

        SECTION("float -> double") {
            constexpr auto src = vector3(3.f, 2.f, 1.f, newton / m2);
            Vector3<pa,double> dst{ 0.f, 0.f, 0.f, pa };
            const auto& res = (dst = src);
            CHECK(&res == &dst);
            CHECK(dst == vector3(3.f, 2.f, 1.f, pa));
        }
    }

    SECTION(".norm()") {
        auto const vec = vector3(1.f, 2.f, -2.f, kg);
        cRealHasRep<float> auto norm = vec.norm();
        CHECK_THAT(norm, matchers::WithinRel(3.f * kg, epsilon));
    }

    SECTION(".dot()") {
        constexpr auto lhs = vector3(1.f, 2.f, 3.f, acc);
        constexpr auto rhs = vector3(1.f, -2.f, 4.f, kg);

        SECTION("// float . float") {
            constexpr cRealHasRep<float> auto res = lhs.dot(rhs);
            CHECK(res == 9.f * newton);
        }

        SECTION("// float . double") {
            constexpr cRealHasRep<double> auto res = Vector3<acc, double>{ lhs }.dot(rhs);
            CHECK(res == 9.f * newton);
        }

        SECTION("// double . float") {
            constexpr cRealHasRep<double> auto res = lhs.dot(Vector3<kg, double>{ rhs });
            CHECK(res == 9.f * newton);
        }
    }

    SECTION("::operator-()") {
        constexpr auto v = vector3(3.f, 2.f, 1.f, newton);
        constexpr cVectorHasRep<float> auto res = -v;
        CHECK(res.unit() == newton);
        CHECK(res == vector3(-3.f, -2.f, -1.f, newton));
    }

    SECTION("") {
        auto lhs = vector3(1.f, 2.f, 3.f, kg);
        auto lhsDouble = Vector3<kg, double>{ lhs };
        auto const rhs = vector3(2.f, 0.f, 5.f, kg);

        SECTION("::operator+=(cVector3ConstArg auto const&)") {
            SECTION("// float += float") {
                cVectorHasRep<float> auto& res = (lhs += rhs);
                CHECK(lhs == vector3(3.f, 2.f, 8.f, kg));
                CHECK(&lhs == &res);
            }

            SECTION("// double += float") {
                cVectorHasRep<double> auto& res = (lhsDouble += rhs);
                CHECK(lhsDouble == vector3(3.f, 2.f, 8.f, kg));
                CHECK(&lhsDouble == &res);
            }
        }

        SECTION("::operator-=(cVector3ConstArg auto const&)") {
            SECTION("// float -= float") {
                cVectorHasRep<float> auto& res = (lhs -= rhs);
                CHECK(lhs == vector3(-1.f, 2.f, -2.f, kg));
                CHECK(&lhs == &res);
            }

            SECTION("// double -= float") {
                cVectorHasRep<double> auto& res = (lhsDouble -= rhs);
                CHECK(lhsDouble == vector3(-1.f, 2.f, -2.f, kg));
                CHECK(&lhsDouble == &res);
            }
        }
    }

    SECTION("// addition & subtraction") {
        constexpr auto lhs = vector3(1.f, 2.f, 3.f, pa);
        constexpr auto lhsDouble = Vector3<pa, double>{ lhs };
        constexpr auto rhs = vector3(5.f, 7.f, -2.f, pa);
        constexpr auto rhsDouble = Vector3<pa, double>{ rhs };

        constexpr auto addResult = vector3(6.f, 9.f, 1.f, pa);
        constexpr auto subResult = vector3(-4.f, -5.f, 5.f, pa);

        SECTION("// vector<float> + vector<float>") {
            constexpr cVectorHasRep<float> auto res = lhs + rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == addResult);
        }

        SECTION("// vector<double> + vector<float>") {
            constexpr cVectorHasRep<double> auto res = lhsDouble + rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == addResult);
        }

        SECTION("// vector<float> + vector<double>") {
            constexpr cVectorHasRep<double> auto res = lhs + rhsDouble;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == addResult);
        }

        SECTION("// vector<float> - vector<float>") {
            constexpr cVectorHasRep<float> auto res = lhs - rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == subResult);
        }

        SECTION("// vector<double> - vector<float>") {
            constexpr cVectorHasRep<double> auto res = lhsDouble - rhs;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == subResult);
        }

        SECTION("// vector<float> - vector<double>") {
            constexpr cVectorHasRep<double> auto res = lhs - rhsDouble;
            CHECK(res.unit() == lhs.unit());
            CHECK(res == subResult);
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

    SECTION("// multiplication") {
        constexpr auto vec = vector3(2.f, -4.f, 3.f, kg);
        constexpr auto vecDouble = Vector3<kg, double>{ vec };

        SECTION("// vector * std::floating_point") {
            constexpr auto expected = vector3(4.f, -8.f, 6.f, kg);

            SECTION("// float * vector<float>") {
                constexpr cVectorHasRep<float> auto res1 = 2.f * vec;
                CHECK(res1 == expected);
                constexpr cVectorHasRep<float> auto res2 = vec * 2.f;
                CHECK(res2 == expected);
            }

            SECTION("// double * vector<float>") {
                constexpr cVectorHasRep<double> auto res1 = 2.0 * vec;
                CHECK(res1 == expected);
                constexpr cVectorHasRep<double> auto res2 = vec * 2.0;
                CHECK(res2 == expected);
            }

            SECTION("// float * vector<double>") {
                constexpr cVectorHasRep<double> auto res1 = 2.f * vecDouble;
                CHECK(res1 == expected);
                constexpr cVectorHasRep<double> auto res2 = vecDouble * 2.f;
                CHECK(res2 == expected);
            }
        }

        SECTION("vector * real") {
            constexpr auto real = 0.5f * acc;
            constexpr auto realDouble = 0.5 * acc;
            constexpr auto expected = vector3(1.f, -2.f, 1.5f, kg * acc);

            SECTION("// Real<float> * vector<float>") {
                constexpr cVectorHasRep<float> auto res1 = real * vec;
                CHECK(res1 == expected);
                constexpr cVectorHasRep<float> auto res2 = vec * real;
                CHECK(res2 == expected);
            }

            SECTION("// Real<float> * vector<double>") {
                constexpr cVectorHasRep<double> auto res1 = real * vecDouble;
                CHECK(res1 == expected);
                constexpr cVectorHasRep<double> auto res2 = vecDouble * real;
                CHECK(res2 == expected);
            }

            SECTION("// Real<double> * vector<float>") {
                constexpr cVectorHasRep<double> auto res1 = realDouble * vec;
                CHECK(res1 == expected);
                constexpr cVectorHasRep<double> auto res2 = vec * realDouble;
                CHECK(res2 == expected);
            }
        }
    }

    SECTION("// division") {
        constexpr auto vec = vector3(2.f, -4.f, 3.f, newton);
        constexpr auto vecDouble = Vector3<newton, double>{ vec };

        SECTION("// vector / float") {
            constexpr auto expected = vector3(1.f, -2.f, 1.5f, newton);

            SECTION("// vector<float> / float") {
                constexpr cVectorHasRep<float> auto res = vec / 2.f;
                CHECK(res == expected);
            }

            SECTION("// vector<double> / float") {
                constexpr cVectorHasRep<double> auto res = vecDouble / 2.f;
                CHECK(res == expected);
            }

            SECTION("// vector<float> / float") {
                constexpr cVectorHasRep<double> auto res = vec / 2.0;
                CHECK(res == expected);
            }
        }

        SECTION("// vector / real") {
            constexpr auto real = 2.f * m2;
            constexpr auto realDouble = 2.0 * m2;
            constexpr auto expected = vector3(1.f, -2.f, 1.5f, pa);

            SECTION("// vector<float> / real<float>") {
                constexpr cVectorHasRep<float> auto res = vec / real;
                CHECK(res == expected);
            }

            SECTION("// vector<double> / real<float>") {
                constexpr cVectorHasRep<double> auto res = vecDouble / real;
                CHECK(res == expected);
            }

            SECTION("// vector<float> / real<double>") {
                constexpr cVectorHasRep<double> auto res = vec / realDouble;
                CHECK(res == expected);
            }
        }
    }

    SECTION("operator==(cVector3 auto const&, cVector3 auto const&)") {
        constexpr auto lhs = vector3(2.f, -3.f, 4.f, pa);

        SECTION("// false") {
            CHECK(lhs != vector3(0.f, -3.f, 4.f, pa));
            CHECK(lhs != vector3(2.f,  0.f, 4.f, pa));
            CHECK(lhs != vector3(2.f, -3.f, 0.f, pa));
        }

        SECTION("//true") {
            CHECK(lhs == vector3(2.f, -3.f, 4.f, pa));
            CHECK(lhs == vector3(2.0, -3.0, 4.0, pa));
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
