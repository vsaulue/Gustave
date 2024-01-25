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

#include <catch2/catch_test_macros.hpp>

#include <gustave/testing/matchers/Vector3Matchers.hpp>

#include <TestConfig.hpp>

TEST_CASE("Vector3WithinRelMatcher") {
    Vector3<u.area> target{1.0, 2.0, -2.0, l2};
    constexpr double epsilon = 0.25;
    const M::Vector3WithinRelMatcher m{ target, epsilon };

    SECTION("::match(cVector3 auto)") {
        CHECK(m.match(Vector3<l2>{1.0, 1.3, -2.0, l2}));
        CHECK_FALSE(m.match(Vector3<l2>{1.0, 1.2, -2.0, l2}));

        CHECK(m.match(1.3 * Vector3<l2>{1.0, 2.0, -2.0, l2}));
        CHECK_FALSE(m.match(1.4 * Vector3<l2>{1.0, 2.0, -2.0, l2}));
    }

    SECTION("::describe()") {
        std::stringstream expected;
        expected << "and '" << target << "' are within " << 100 * epsilon << "% of each other.";
        CHECK(m.describe() == expected.str());
    }
}

TEST_CASE("Vector3WithinRel(cVector3 auto, std::floating_point auto)") {
    CHECK_THAT(Vector3<l2>(1.0, 0.0, 0.0, l2), M::WithinRel(Vector3<l2>(0.8, 0.0, 0.0, l2), 0.25));
    CHECK_THAT(Vector3<l2>(1.0, 0.0, 0.0, l2), !M::WithinRel(Vector3<l2>(0.7, 0.0, 0.0, l2), 0.25));
}
