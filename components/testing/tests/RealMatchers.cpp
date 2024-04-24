/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <gustave/testing/matchers/RealMatchers.hpp>

#include <TestConfig.hpp>

TEST_CASE("RealWithinRelMatcher") {
    constexpr Real<u.area> target = -2.0 * l2;
    constexpr double epsilon = 0.25;
    const matchers::RealWithinRelMatcher m{ target, epsilon };

    SECTION("::match(cReal auto)") {
        CHECK(m.match(-1.6 * l2));
        CHECK_FALSE(m.match(-1.4 * l2));

        CHECK(m.match(-2.6 * l2));
        CHECK_FALSE(m.match(-2.9 * l2));
    }

    SECTION("::describe()") {
        std::stringstream expected;
        expected << "and '" << target << "' are within " << 100 * epsilon << "% of each other.";
        CHECK(m.describe() == expected.str());
    }
}

TEST_CASE("WithinRel(cReal auto, std::floating_point auto)") {
    CHECK_THAT(1.6 * l2, matchers::WithinRel(2.0 * l2, 0.25));
    CHECK_THAT(1.4 * l2, !matchers::WithinRel(2.0 * l2, 0.25));
}
