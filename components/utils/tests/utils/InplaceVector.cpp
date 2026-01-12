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

#include <array>
#include <cstdint>
#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <gustave/testing/DelDetector.hpp>
#include <gustave/utils/Prop.hpp>
#include <gustave/utils/InplaceVector.hpp>

namespace utils = gustave::utils;

using DelDetector = gustave::testing::DelDetector;

using IntVector = utils::InplaceVector<int, 4>;
using SharedVector = utils::InplaceVector<utils::prop::SharedPtr<DelDetector>, 3>;
using UniqueVector = utils::InplaceVector<std::unique_ptr<DelDetector>, 5>;

static_assert(std::ranges::random_access_range<utils::InplaceVector<int,4>>);

// Test "::isConstCopyable()"
static_assert(IntVector::isConstCopyable());
static_assert(!SharedVector::isConstCopyable());
static_assert(!UniqueVector::isConstCopyable());

// Test "::isMoveCopyable()"
static_assert(IntVector::isMoveCopyable());
static_assert(SharedVector::isMoveCopyable());
static_assert(UniqueVector::isMoveCopyable());

// Test "::isNonConstCopyable()"
static_assert(IntVector::isNonConstCopyable());
static_assert(SharedVector::isNonConstCopyable());
static_assert(!UniqueVector::isNonConstCopyable());

TEST_CASE("utils::InplaceVector") {
    auto intVec = IntVector{};
    auto const& cIntVec = intVec;
    intVec.pushBack(5);
    intVec.pushBack(10);
    intVec.pushBack(15);

    auto sharedVec = SharedVector{};
    bool sDelFlag0 = false;
    bool sDelFlag1 = false;
    sharedVec.pushBack(std::make_shared<DelDetector>(sDelFlag0));
    sharedVec.pushBack(std::make_shared<DelDetector>(sDelFlag1));

    auto uniqueVec = UniqueVector{};
    bool uDelFlag0 = false;
    bool uDelFlag1 = false;
    uniqueVec.pushBack(std::make_unique<DelDetector>(uDelFlag0));
    uniqueVec.pushBack(std::make_unique<DelDetector>(uDelFlag1));

    SECTION("::InplaceVector(InplaceVector const&) // const copy") {
        auto copy = IntVector{ cIntVec };
        intVec.at(1) = 7;
        auto const expected = std::array{ 5,10,15 };
        REQUIRE(copy.size() == 3);
        CHECK_THAT(copy, Catch::Matchers::RangeEquals(expected));
    }

    SECTION("::InplaceVector(InplaceVector&) // non-const copy") {
        auto copy = SharedVector{ sharedVec };
        REQUIRE_FALSE(sDelFlag0);
        REQUIRE_FALSE(sDelFlag1);
        REQUIRE(copy.size() == 2);
        REQUIRE(sharedVec.size() == 2);
        CHECK(sharedVec.at(0) == copy.at(0));
        CHECK(sharedVec.at(1) == copy.at(1));
    }

    SECTION("::InplaceVector(InplaceVector&&) // move copy") {
        auto const a0 = uniqueVec.at(0).get();
        auto const a1 = uniqueVec.at(1).get();
        auto copy = UniqueVector{ std::move(uniqueVec) };
        REQUIRE_FALSE(uDelFlag0);
        REQUIRE_FALSE(uDelFlag1);
        CHECK(uniqueVec.size() == 0);
        REQUIRE(copy.size() == 2);
        CHECK(copy.at(0).get() == a0);
        CHECK(copy.at(1).get() == a1);
    }

    SECTION("::~InplaceVector() // destructor") {
        bool vDelFlag0 = false;
        bool vDelFlag1 = false;
        {
            auto vec = utils::InplaceVector<DelDetector, 3>{};
            vec.emplaceBack(vDelFlag0);
            vec.emplaceBack(vDelFlag1);
        }
        CHECK(vDelFlag0);
        CHECK(vDelFlag1);
    }

    SECTION(".operator=(InplaceVector const&) // const assign") {
        auto newVec = IntVector{};
        newVec.pushBack(38);
        newVec.pushBack(72);
        intVec = newVec;
        newVec.at(0) = 45;
        auto const expected = std::array{ 38,72 };
        REQUIRE(intVec.size() == 2);
        CHECK_THAT(intVec, Catch::Matchers::RangeEquals(expected));
    }

    SECTION(".operator=(InplaceVector&) // non-const assign") {
        auto newVec = SharedVector{};
        bool nDelFlag0 = false;
        newVec.pushBack(std::make_shared<DelDetector>(nDelFlag0));
        sharedVec = newVec;
        REQUIRE(sDelFlag0);
        REQUIRE(sDelFlag1);
        REQUIRE_FALSE(nDelFlag0);
        REQUIRE(sharedVec.size() == 1);
        REQUIRE(newVec.size() == 1);
        CHECK(sharedVec.at(0) == newVec.at(0));
    }

    SECTION(".operator=(InplaceVector&&) // move assign") {
        auto newVec = UniqueVector{};
        bool nDelFlag0 = false;
        newVec.pushBack(std::make_unique<DelDetector>(nDelFlag0));
        auto const a0 = newVec.at(0).get();
        uniqueVec = std::move(newVec);
        REQUIRE(uDelFlag0);
        REQUIRE(uDelFlag1);
        REQUIRE_FALSE(nDelFlag0);
        REQUIRE(uniqueVec.size() == 1);
        REQUIRE(newVec.size() == 0);
        CHECK(uniqueVec.at(0).get() == a0);
    }

    SECTION(".at()") {
        SECTION("// valid") {
            auto& val = intVec.at(1);
            auto const& cVal = cIntVec.at(1);
            CHECK(val == 10);
            CHECK(cVal == 10);
            val = 12;
            CHECK(cVal == 12);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(intVec.at(3), std::out_of_range);
            CHECK_THROWS_AS(cIntVec.at(4), std::out_of_range);
        }
    }

    SECTION(".begin() // & .end()") {
        SECTION("// const") {
            auto const expected = std::array{ 5,10,15 };
            CHECK_THAT(cIntVec, Catch::Matchers::RangeEquals(expected));
        }

        SECTION("// mutable") {
            int i = 1;
            for (auto& val : intVec) {
                val += i;
                ++i;
            }
            auto const expected = std::array{ 6,12,18 };
            CHECK_THAT(cIntVec, Catch::Matchers::RangeEquals(expected));
        }
    }

    SECTION(".clear()") {
        sharedVec.clear();
        CHECK(sharedVec.size() == 0);
        CHECK(sDelFlag0);
        CHECK(sDelFlag1);
    }

    SECTION(".emplaceBack()") {
        auto vec = utils::InplaceVector<DelDetector, 3>{};
        bool vDelFlag0 = false;
        bool vDelFlag1 = false;
        vec.emplaceBack(vDelFlag0);
        vec.emplaceBack(vDelFlag1);
        REQUIRE(vec.size() == 2);
        CHECK(&vec.at(0).delFlag() == &vDelFlag0);
        CHECK(&vec.at(1).delFlag() == &vDelFlag1);
    }

    SECTION(".operator[]") {
        auto& val = intVec[2];
        auto const& cVal = cIntVec[2];
        CHECK(val == 15);
        CHECK(cVal == 15);
        val = 3;
        CHECK(cVal == 3);
    }

    SECTION(".pushBack(Value const&) // const copy") {
        auto const nVal = 38;
        intVec.pushBack(nVal);
        REQUIRE(intVec.size() == 4);
        CHECK(intVec.at(3) == nVal);
    }

    SECTION(".pushBack(Value&) // non-const copy") {
        bool nDelFlag = false;
        auto nVal = utils::prop::SharedPtr<DelDetector>{ std::make_shared<DelDetector>(nDelFlag) };
        sharedVec.pushBack(nVal);
        CHECK_FALSE(nDelFlag);
        REQUIRE(sharedVec.size() == 3);
        CHECK(sharedVec.at(2) == nVal);
    }

    SECTION(".pushBack(Value&&) // move copy") {
        bool nDelFlag = false;
        auto nVal = std::make_unique<DelDetector>(nDelFlag);
        auto const nAddr = nVal.get();
        uniqueVec.pushBack(std::move(nVal));
        CHECK_FALSE(nDelFlag);
        REQUIRE(uniqueVec.size() == 3);
        CHECK(uniqueVec.at(2).get() == nAddr);
    }

    SECTION(".size()") {
        CHECK(cIntVec.size() == 3);
    }
}
