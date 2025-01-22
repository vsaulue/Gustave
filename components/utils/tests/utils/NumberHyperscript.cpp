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

#include <cstdint>
#include <limits>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "gustave/utils/NumberHyperscript.hpp"

namespace utils = gustave::utils;

TEST_CASE("utils::NumberHyperscript") {
    SECTION("::NumberHyperscript(std::integral)") {
        SECTION("// int64: min") {
            constexpr utils::NumberHyperscript obj{ std::numeric_limits<std::int64_t>::min() };
            REQUIRE(obj.view() == "⁻⁹²²³³⁷²⁰³⁶⁸⁵⁴⁷⁷⁵⁸⁰⁸");
        }

        SECTION("// int32: -1,111,111,111") {
            constexpr utils::NumberHyperscript obj{ std::int32_t{ -1111111111 } };
            REQUIRE(obj.view() == "⁻¹¹¹¹¹¹¹¹¹¹");
        }

        SECTION("// int32_t: 0") {
            constexpr utils::NumberHyperscript<std::int32_t> obj{ 0 };
            REQUIRE(obj.view() == "⁰");
        }

        SECTION("// uint64_t: max") {
            constexpr utils::NumberHyperscript obj{ std::numeric_limits<std::uint64_t>::max() };
            REQUIRE(obj.view() == "¹⁸⁴⁴⁶⁷⁴⁴⁰⁷³⁷⁰⁹⁵⁵¹⁶¹⁵");
        }
    }

    SECTION("") {
        constexpr utils::NumberHyperscript obj{ 12345 };
        const std::string expected = "¹²³⁴⁵";

        SECTION("::view()") {
            REQUIRE(obj.view() == expected);
        }

        SECTION("::operator std::string_view()") {
            REQUIRE(std::string_view{ obj } == expected);
        }

        SECTION("::begin() // & end()") {
            std::string copy = { obj.begin(), obj.end() };
            REQUIRE(copy == expected);
        }

        SECTION("::size()") {
            REQUIRE(obj.size() == expected.size());
        }
    }
}
