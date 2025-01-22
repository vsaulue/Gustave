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

#include <TestHelpers.hpp>

#include <gustave/cfg/cRealTraits.hpp>

static_assert(cfg::cRealTraits<Rt>);

static void checkValue(auto real, std::floating_point auto expected) {
    CHECK(std::is_same_v<typename decltype(real)::Rep, decltype(expected)>);
    CHECK(real.value() == expected);
}

TEST_CASE("Real") {
    SECTION("std::floating_point * Unit -> Real") {
        SECTION("// float") {
            auto real = 5.f * u.length;
            checkValue(real, 5.f);
            CHECK(real.unit() == u.length);
        }

        SECTION("// double") {
            auto real = 3.0 * u.area;
            checkValue(real, 3.0);
            CHECK(real.unit() == u.area);
        }
    }

    SECTION("std::floating_point / Unit -> Real") {
        SECTION("// float") {
            auto real = 5.f / u.length;
            checkValue(real, 5.f);
            CHECK((real.unit() * u.length).isOne());
        }

        SECTION("// double") {
            auto real = 2.0 / u.force;
            checkValue(real, 2.0);
            CHECK((real.unit() * u.force).isOne());
        }
    }

    SECTION("Real{ Real }") {
        SECTION("Real<float>{ Real<float> }") {
            auto r1 = Real<u.area, float>{ 4.f * (u.length * u.length) };
            checkValue(r1, 4.f);
            CHECK(r1.unit() == u.area);
        }

        SECTION("Real<double>{ Real<float> }") {
            auto r1 = Real<u.area, double>{ 4.f * (u.length * u.length) };
            checkValue(r1, 4.0);
            CHECK(r1.unit() == u.area);
        }
    }

    SECTION("Real{ std::floating_point, Unit }") {
        auto r1 = Real<u.area, float>{ 2.f, u.length * u.length };
        checkValue(r1, 2.f);
        CHECK(r1.unit() == u.area);
    }

    SECTION("Real{ std::floating_point }") {
        auto r1 = Real<u.one, float>{ 5.f };
        checkValue(r1, 5.f);
        CHECK(r1.unit() == u.one);
    }

    SECTION("::zero()") {
        auto real = Real<u.force, float>::zero();
        checkValue(real, 0.f);
        CHECK(real.unit() == u.force);
    }

    SECTION("Real = Real") {
        SECTION("Real<float> = Real<float>") {
            auto lhs = 5.f * u.area;
            auto& res = (lhs = 10.f * (u.length * u.length));
            checkValue(lhs, 10.f);
            CHECK(&lhs == &res);
        }

        SECTION("Real<double> = Real<float>") {
            auto lhs = 5.0 * u.area;
            auto& res = (lhs = 10.f * (u.length * u.length));
            checkValue(lhs, 10.0);
            CHECK(&lhs == &res);
        }
    }

    SECTION("Real = std::floating_point") {
        SECTION("Real<float> = float") {
            auto lhs = 5.f * u.one;
            auto& res = (lhs = 10.f);
            checkValue(lhs, 10.f);
            CHECK(&lhs == &res);
        }

        SECTION("Real<double> = float") {
            auto lhs = 5.0 * u.one;
            auto& res = (lhs = 10.f);
            checkValue(lhs, 10.0);
            CHECK(&lhs == &res);
        }
    }

    SECTION("-Real") {
        auto real = -(2.5 * u.conductivity);
        checkValue(real, -2.5);
        CHECK(real.unit() == u.conductivity);
    }

    // Equality

    SECTION("Real == Real") {
        SECTION("Real<float> == Real<float>") {
            CHECK(3.f * u.acceleration == 3.f * u.acceleration);
            CHECK(5.f * u.force != 4.f * u.force);
        }

        SECTION("Real<double> == Real<float>") {
            CHECK(3.0 * u.acceleration == 3.f * u.acceleration);
            CHECK(5.0 * u.force != 4.f * u.force);
        }

        SECTION("Real<float> == Real<double>") {
            CHECK(3.f * u.acceleration == 3.0 * u.acceleration);
            CHECK(5.f * u.force != 4.0 * u.force);
        }
    }

    SECTION("Real == std::floating_point") {
        SECTION("Real<float> == float") {
            CHECK(3.f * u.one == 3.f);
            CHECK(5.f * u.one != 4.f);
        }

        SECTION("Real<double> == float") {
            CHECK(3.0 * u.one == 3.f);
            CHECK(5.0 * u.one != 4.f);
        }

        SECTION("Real<float> == double") {
            CHECK(3.f * u.one == 3.0);
            CHECK(5.f * u.one != 4.0);
        }
    }

    SECTION("std::floating_point == Real") {
        SECTION("float == Real<float>") {
            CHECK(3.f == 3.f * u.one);
            CHECK(5.f != 4.f * u.one);
        }

        SECTION("double == Real<float>") {
            CHECK(3.0 == 3.f * u.one);
            CHECK(5.0 != 4.f * u.one);
        }

        SECTION("float == Real<double>") {
            CHECK(3.f == 3.0 * u.one);
            CHECK(5.f != 4.0 * u.one);
        }
    }

    // Comparison

    SECTION("Real <=> Real") {
        SECTION("Real<float> <=> Real<float>") {
            CHECK(5.f * u.area > 4.f * (u.length * u.length));
        }

        SECTION("Real<double> <=> Real<float>") {
            CHECK(5.0 * u.area > 4.f * (u.length * u.length));
        }

        SECTION("Real<float> <=> Real<double>") {
            CHECK(4.f * u.area < 5.0 * (u.length * u.length));
        }
    }

    SECTION("Real <=> std::floating_point") {
        SECTION("Real<float> <=> float") {
            CHECK(5.f * u.one > 4.f);
        }

        SECTION("Real<double> <=> float") {
            CHECK(5.0 * u.one > 4.f);
        }

        SECTION("Real<float> <=> double") {
            CHECK(4.f * u.one < 5.0);
        }
    }

    SECTION("std::floating_point <=> Real") {
        SECTION("float <=> Real<float>") {
            CHECK(5.f > 4.f * u.one);
        }

        SECTION("double <=> Real<float>") {
            CHECK(5.0 > 4.f * u.one);
        }

        SECTION("float <=> Real<double>") {
            CHECK(4.f < 5.0 * u.one);
        }
    }

    // operator += 

    SECTION("Real += Real") {
        SECTION("Real<float> += Real<float>") {
            auto lhs = 5.f * u.force;
            auto& res = (lhs += 4.f * u.force);
            CHECK(&res == &lhs);
            checkValue(lhs, 9.f);
        }

        SECTION("Real<double> += Real<float>") {
            auto lhs = 5.0 * u.force;
            auto& res = (lhs += 4.f * u.force);
            CHECK(&res == &lhs);
            checkValue(lhs, 9.0);
        }
    }

    SECTION("Real += std::floating_point") {
        SECTION("Real<float> += float") {
            auto lhs = 5.f * u.one;
            auto& res = (lhs += 4.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 9.f);
        }

        SECTION("Real<double> += float") {
            auto lhs = 5.0 * u.one;
            auto& res = (lhs += 4.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 9.0);
        }
    }

    // operator -=

    SECTION("Real -= Real") {
        SECTION("Real<float> -= Real<float>") {
            auto lhs = 5.f * u.force;
            auto& res = (lhs -= 4.f * u.force);
            CHECK(&res == &lhs);
            checkValue(lhs, 1.f);
        }

        SECTION("Real<double> -= Real<float>") {
            auto lhs = 5.0 * u.force;
            auto& res = (lhs -= 4.f * u.force);
            CHECK(&res == &lhs);
            checkValue(lhs, 1.0);
        }
    }

    SECTION("Real -= std::floating_point") {
        SECTION("Real<float> -= float") {
            auto lhs = 5.f * u.one;
            auto& res = (lhs -= 4.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 1.f);
        }

        SECTION("Real<double> -= float") {
            auto lhs = 5.0 * u.one;
            auto& res = (lhs -= 4.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 1.0);
        }
    }

    // operator *=

    SECTION("Real *= Real") {
        SECTION("Real<float> *= Real<float>") {
            auto lhs = 2.f * u.length;
            auto& res = (lhs *= (3.f * u.one));
            CHECK(&res == &lhs);
            checkValue(lhs, 6.f);
        }

        SECTION("Real<double> *= Real<float>") {
            auto lhs = 2.0 * u.length;
            auto& res = (lhs *= (3.f * u.one));
            CHECK(&res == &lhs);
            checkValue(lhs, 6.0);
        }
    }

    SECTION("Real *= std::floating_point") {
        SECTION("Real<float> *= float") {
            auto lhs = 2.f * u.length;
            auto& res = (lhs *= 3.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 6.f);
        }

        SECTION("Real<double> *= Real<float>") {
            auto lhs = 2.0 * u.length;
            auto& res = (lhs *= 3.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 6.0);
        }
    }

    // operator/=

    SECTION("Real /= Real") {
        SECTION("Real<float> /= Real<float>") {
            auto lhs = 8.f * u.length;
            auto& res = (lhs /= (4.f * u.one));
            CHECK(&res == &lhs);
            checkValue(lhs, 2.f);
        }

        SECTION("Real<double> /= Real<float>") {
            auto lhs = 8.0 * u.length;
            auto& res = (lhs /= (4.f * u.one));
            CHECK(&res == &lhs);
            checkValue(lhs, 2.0);
        }
    }

    SECTION("Real /= std::floating_point") {
        SECTION("Real<float> /= float") {
            auto lhs = 8.f * u.length;
            auto& res = (lhs /= 4.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 2.f);
        }

        SECTION("Real<double> /= float") {
            auto lhs = 8.0 * u.length;
            auto& res = (lhs /= 4.f);
            CHECK(&res == &lhs);
            checkValue(lhs, 2.0);
        }
    }

    // Addition

    SECTION("Real + Real") {
        SECTION("Real<float> + Real<float>") {
            auto res = 2.f * u.force + 3.f * u.force;
            checkValue(res, 5.f);
            CHECK(res.unit() == u.force);
        }

        SECTION("Real<float> + Real<double>") {
            auto res = 2.f * u.force + 3.0 * u.force;
            checkValue(res, 5.0);
            CHECK(res.unit() == u.force);
        }

        SECTION("Real<double> + Real<float>") {
            auto res = 2.0 * u.force + 3.f * u.force;
            checkValue(res, 5.0);
            CHECK(res.unit() == u.force);
        }
    }

    SECTION("Real + std::floating_point") {
        SECTION("Real<float> + float") {
            auto res = 2.f * u.one + 1.f;
            checkValue(res, 3.f);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<float> + double") {
            auto res = 2.f * u.one + 1.0;
            checkValue(res, 3.0);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<double> + float") {
            auto res = 2.0 * u.one + 1.f;
            checkValue(res, 3.0);
            CHECK(res.unit() == u.one);
        }
    }

    SECTION("std::floating_point + Real") {
        SECTION("Real<float> + float") {
            auto res = 2.f + 4.f * u.one;
            checkValue(res, 6.f);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<float> + double") {
            auto res = 2.f + 4.0 * u.one;
            checkValue(res, 6.0);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<double> + float") {
            auto res = 2.0 + 4.f * u.one;
            checkValue(res, 6.0);
            CHECK(res.unit() == u.one);
        }
    }

    // Subtraction

    SECTION("Real - Real") {
        SECTION("Real<float> - Real<float>") {
            auto res = 2.f * u.force - 3.f * u.force;
            checkValue(res, -1.f);
            CHECK(res.unit() == u.force);
        }

        SECTION("Real<float> + Real<double>") {
            auto res = 2.f * u.force - 3.0 * u.force;
            checkValue(res, -1.0);
            CHECK(res.unit() == u.force);
        }

        SECTION("Real<double> + Real<float>") {
            auto res = 2.0 * u.force - 3.f * u.force;
            checkValue(res, -1.0);
            CHECK(res.unit() == u.force);
        }
    }

    SECTION("Real - std::floating_point") {
        SECTION("Real<float> - float") {
            auto res = 2.f * u.one - 1.f;
            checkValue(res, 1.f);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<float> - double") {
            auto res = 2.f * u.one - 1.0;
            checkValue(res, 1.0);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<double> - float") {
            auto res = 2.0 * u.one - 1.f;
            checkValue(res, 1.0);
            CHECK(res.unit() == u.one);
        }
    }

    SECTION("std::floating_point - Real") {
        SECTION("Real<float> - float") {
            auto res = 2.f - 4.f * u.one;
            checkValue(res, -2.f);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<float> - double") {
            auto res = 2.f - 4.0 * u.one;
            checkValue(res, -2.0);
            CHECK(res.unit() == u.one);
        }

        SECTION("Real<double> - float") {
            auto res = 2.0 - 4.f * u.one;
            checkValue(res, -2.0);
            CHECK(res.unit() == u.one);
        }
    }

    // Multiplication

    SECTION("Real * Real") {
        SECTION("Real<float> * Real<float>") {
            auto res = (1.5f * u.mass) * (4.f * u.acceleration);
            checkValue(res, 6.f);
            CHECK(res.unit().isAssignableFrom(u.force));
        }

        SECTION("Real<float> * Real<double>") {
            auto res = (1.5f * u.mass) * (4.0 * u.acceleration);
            checkValue(res, 6.0);
            CHECK(res.unit().isAssignableFrom(u.force));
        }

        SECTION("Real<double> * Real<float>") {
            auto res = (1.5 * u.mass) * (4.f * u.acceleration);
            checkValue(res, 6.0);
            CHECK(res.unit().isAssignableFrom(u.force));
        }
    }

    SECTION("Real * std::floating_point") {
        SECTION("Real<float> * float") {
            auto res = (1.5f * u.mass) * 4.f;
            checkValue(res, 6.f);
            CHECK(res.unit().isAssignableFrom(u.mass));
        }

        SECTION("Real<float> * double") {
            auto res = (1.5f * u.mass) * 4.0;
            checkValue(res, 6.0);
            CHECK(res.unit().isAssignableFrom(u.mass));
        }

        SECTION("Real<double> * float") {
            auto res = (1.5 * u.mass) * 4.f;
            checkValue(res, 6.0);
            CHECK(res.unit().isAssignableFrom(u.mass));
        }
    }

    SECTION("std::floating_point * Real") {
        SECTION("float * Real<float>") {
            auto res = 1.5f * (4.f * u.area);
            checkValue(res, 6.f);
            CHECK(res.unit() == u.area);
        }

        SECTION("float * Real<double>") {
            auto res = 1.5f * (4.0 * u.area);
            checkValue(res, 6.0);
            CHECK(res.unit() == u.area);
        }

        SECTION("double * Real<float>") {
            auto res = 1.5 * (4.f * u.area);
            checkValue(res, 6.0);
            CHECK(res.unit() == u.area);
        }
    }

    // Division

    SECTION("Real / Real") {
        SECTION("Real<float> / Real<float>") {
            auto res = (6.f * u.area) / (3.f * u.length);
            checkValue(res, 2.f);
            CHECK(res.unit().isAssignableFrom(u.length));
        }

        SECTION("Real<float> / Real<double>") {
            auto res = (6.f * u.area) / (3.0 * u.length);
            checkValue(res, 2.0);
            CHECK(res.unit().isAssignableFrom(u.length));
        }

        SECTION("Real<double> / Real<float>") {
            auto res = (6.0 * u.area) / (3.f * u.length);
            checkValue(res, 2.0);
            CHECK(res.unit().isAssignableFrom(u.length));
        }
    }

    SECTION("Real / std::floating_point") {
        SECTION("Real<float> / float") {
            auto res = (6.f * u.area) / 3.f;
            checkValue(res, 2.f);
            CHECK(res.unit() == u.area);
        }

        SECTION("Real<float> / double") {
            auto res = (6.f * u.area) / 3.0;
            checkValue(res, 2.0);
            CHECK(res.unit() == u.area);
        }

        SECTION("Real<double> / float") {
            auto res = (6.0 * u.area) / 3.f;
            checkValue(res, 2.0);
            CHECK(res.unit() == u.area);
        }
    }

    SECTION("std::floating_point / Real") {
        SECTION("float / Real<float>") {
            auto res = 6.f / (3.f * u.length);
            checkValue(res, 2.f);
            CHECK((res.unit() * u.length).isOne());
        }

        SECTION("float / Real<double>") {
            auto res = 6.f / (3.0 * u.length);
            checkValue(res, 2.0);
            CHECK((res.unit() * u.length).isOne());
        }

        SECTION("double / Real<float>") {
            auto res = 6.0 / (3.f * u.length);
            checkValue(res, 2.0);
            CHECK((res.unit() * u.length).isOne());
        }
    }
}
