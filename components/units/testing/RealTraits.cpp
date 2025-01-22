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

static void checkReal(auto value, auto expected) {
    CHECK(value == expected);
    CHECK(std::is_same_v<typename decltype(value)::Rep, typename decltype(expected)::Rep>);
}

namespace {
    struct FakeUnit {};
}

TEST_CASE("RealTraits") {
    SECTION(".abs()") {
        SECTION("// Real<float>") {
            auto value = rt.abs(100.f * u.area);
            checkReal(value, 100.f * u.area);
        }

        SECTION("// Real<double>") {
            auto value = rt.abs(-100.0 * u.length);
            checkReal(value, 100.0 * u.length);
        }
    }

    SECTION(".isUnit()") {
        SECTION("// true") {
            CHECK(rt.isUnit(u.length));
        }

        SECTION("// false") {
            CHECK_FALSE(rt.isUnit(FakeUnit{}));
        }
    }

    SECTION(".max()") {
        SECTION("(Real<float>, Real<float>)") {
            auto r1 = 5.f * u.area;
            auto r2 = 4.f * (u.length * u.length);
            auto res = rt.max(r1, r2);
            checkReal(res, 5.f * u.area);
        }

        SECTION("(Real<double>, Real<float>)") {
            auto r1 = 5.0 * u.area;
            auto r2 = 4.f * (u.length * u.length);
            auto res = rt.max(r1, r2);
            checkReal(res, 5.0 * u.area);
        }

        SECTION("(Real<float>, Real<double>)") {
            auto r1 = 4.f * u.area;
            auto r2 = 5.0 * (u.length * u.length);
            auto res = rt.max(r1, r2);
            checkReal(res, 5.0 * u.area);
        }
    }

    SECTION(".min()") {
        SECTION("(Real<float>, Real<float>)") {
            auto r1 = 5.f * u.area;
            auto r2 = 4.f * (u.length * u.length);
            auto res = rt.min(r1, r2);
            checkReal(res, 4.f * u.area);
        }

        SECTION("(Real<double>, Real<float>)") {
            auto r1 = 5.0 * u.area;
            auto r2 = 4.f * (u.length * u.length);
            auto res = rt.min(r1, r2);
            checkReal(res, 4.0 * u.area);
        }

        SECTION("(Real<float>, Real<double>)") {
            auto r1 = 4.f * u.area;
            auto r2 = 5.0 * (u.length * u.length);
            auto res = rt.min(r1, r2);
            checkReal(res, 4.0 * u.area);
        }
    }

    SECTION(".signBit()") {
        SECTION("// negative") {
            CHECK(rt.signBit(-1.f * u.area));
        }

        SECTION("// positive") {
            CHECK_FALSE(rt.signBit(0.5 * u.length));
        }
    }

    SECTION(".sqrt()") {
        SECTION("// Real<float>") {
            auto value = rt.sqrt(100.f * u.area);
            checkReal(value, 10.f * u.length);
        }

        SECTION("// Real<double>") {
            auto value = rt.sqrt(100.0 * u.area);
            checkReal(value, 10.0 * u.length);
        }
    }
}
