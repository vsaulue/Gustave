/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <sstream>
#include <string_view>

#include "gustave/utils/SizedString.hpp"
#include "gustave/utils/SizedStringView.hpp"

using namespace std::string_view_literals;

namespace utils = gustave::utils;

static_assert(::Catch::Detail::IsStreamInsertable<std::string_view>::value);

TEST_CASE("utils::SizedString") {
    SECTION("// empty string") {
        constexpr utils::SizedString<char,0> str = "";
        REQUIRE(str == ""sv);
    }

    SECTION("::SizedString(SizedStringView)") {
        constexpr utils::SizedStringView<char, 4> sizedView = "abcde";
        constexpr utils::SizedString str{ sizedView };
        REQUIRE(str == "abcd"sv);
    }

	SECTION("::SizedString(const char[])") {
		constexpr auto str = utils::SizedString("Hello world");
		REQUIRE(str == "Hello world"sv);
	}

	SECTION("::SizedString(SizedStringView...)") {
		constexpr utils::SizedStringView str1{ "Hello" };
		constexpr utils::SizedStringView str2{ "World" };
		constexpr utils::SizedStringView str3{ "!!!" };
		constexpr utils::SizedString str{ str1, str2, str3};
		REQUIRE(str == "HelloWorld!!!"sv);
	}

    SECTION("SizedString == SizedString") {
        SECTION("// true") {
            constexpr utils::SizedString lhs = "abcd0";
            constexpr utils::SizedString rhs = "abcd0";
            REQUIRE(lhs == rhs);
        }

        SECTION("// false") {
            constexpr utils::SizedString lhs = "abcd00";
            constexpr utils::SizedString rhs = "abcd0";
            REQUIRE(lhs != rhs);
        }
    }

	SECTION("::operator<=>(cSizedString auto, cSizedString auto)") {
		constexpr utils::SizedString lhs = "abcd0";
		constexpr utils::SizedString rhs = "abcd9";
		REQUIRE(lhs < rhs);
	}

    SECTION("operator<<(std::basic_ostream&, cSizedString auto)") {
        std::stringstream stream;

        SECTION("// non trivial string") {
            constexpr utils::SizedString str = "¹⁸⁴⁴⁶";
            stream << str;
            CHECK(stream.view() == "¹⁸⁴⁴⁶"sv);
        }

        SECTION("// empty string") {
            constexpr utils::SizedString<char, 0> str = "";
            stream << str;
            CHECK(stream.view() == ""sv);
        }
    }
}
