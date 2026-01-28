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

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <gustave/utils/SharedIndexedSet.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <Point.hpp>

namespace utils = gustave::utils;

using ConstDetector = gustave::testing::ConstDetector<int>;

namespace {
    class Item : public ConstDetector {
    public:
        [[nodiscard]]
        explicit Item(Point const& index, int tag)
            : ConstDetector(tag)
            , index_{index}
        {}

        [[nodiscard]]
        Point const& index() const {
            return index_;
        }
    private:
        Point index_;
    };
}

using Set = utils::SharedIndexedSet<Item>;

template<typename T>
using SharedPtr = Set::SharedPtr<T>;

static_assert(std::ranges::forward_range<Set>);

TEST_CASE("utils::SharedIndexedSet") {
    auto set = Set{};
    auto const& cSet = set;
    auto& i012 = set.emplace(Point{ 0,1,2 }, 12);
    auto& i505 = set.emplace(Point{ 5,0,5 }, 505);

    SECTION(".at()") {
        SECTION("// valid - const") {
            auto& res = cSet.at({0,1,2});
            REQUIRE(&res == &i012);
            CHECK(res.tag == 12);
            CHECK(res.isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto& res = set.at({5,0,5});
            REQUIRE(&res == &i505);
            CHECK(res.tag == 505);
            CHECK_FALSE(res.isCalledAsConst());
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(set.at({5,0,4}), std::out_of_range);
            CHECK_THROWS_AS(cSet.at({1,0,2}), std::out_of_range);
        }
    }

    SECTION(".atShared()") {
        SECTION("// valid - const") {
            auto res = cSet.atShared({ 5,0,5 });
            REQUIRE(res.get() == &i505);
            CHECK(res->isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto res = set.atShared({ 0,1,2 });
            REQUIRE(res.get() == &i012);
            CHECK_FALSE(res->isCalledAsConst());
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(set.atShared({2,0,1}), std::out_of_range);
            CHECK_THROWS_AS(cSet.atShared({4,0,5}), std::out_of_range);
        }
    }

    SECTION(".begin() // &.end()") {
        auto const expected = std::array{ set.atShared({0,1,2}), set.atShared({5,0,5}) };

        SECTION("// mutable") {
            REQUIRE_THAT(set, Catch::Matchers::UnorderedRangeEquals(expected));
            auto const it = set.begin();
            CHECK_FALSE((*it)->isCalledAsConst());
        }

        SECTION("// const") {
            REQUIRE_THAT(cSet, Catch::Matchers::UnorderedRangeEquals(expected));
            auto const it = cSet.begin();
            CHECK((*it)->isCalledAsConst());
        }
    }

    SECTION(".contains()") {
        SECTION("// true") {
            CHECK(cSet.contains({ 0,1,2 }));
        }

        SECTION("// false") {
            CHECK_FALSE(cSet.contains({ 0,1,1 }));
        }
    }

    SECTION(".emplace()") {
        auto& res = set.emplace(Point{ 4,5,6 }, 456);
        CHECK(cSet.size() == 3);
        CHECK(&cSet.at({ 4,5,6 }) == &res);
    }

    SECTION(".erase()") {
        SECTION("// present") {
            auto const res = set.erase({ 0,1,2 });
            CHECK(res);
            CHECK(cSet.size() == 1);
        }

        SECTION("// absent") {
            auto const res = set.erase({ 0,2,2 });
            CHECK_FALSE(res);
            CHECK(cSet.size() == 2);
        }
    }

    SECTION(".extract()") {
        SECTION("// present") {
            auto res = set.extract({ 5,0,5 });
            REQUIRE(res);
            CHECK(res->tag == 505);
            CHECK(cSet.size() == 1);
        }

        SECTION("// absent") {
            auto res = set.extract({ 0,1,1 });
            CHECK_FALSE(res);
            CHECK(cSet.size() == 2);
        }
    }

    SECTION(".find()") {
        SECTION("// valid - const") {
            auto res = cSet.find({ 5,0,5 });
            REQUIRE(res == &i505);
            CHECK(res->isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto res = set.find({ 0,1,2 });
            REQUIRE(res == &i012);
            CHECK_FALSE(res->isCalledAsConst());
        }

        SECTION("// invalid") {
            auto const res = set.find({ 0,0,0 });
            CHECK(res == nullptr);
        }
    }

    SECTION(".findShared()") {
        SECTION("// valid - const") {
            auto res = cSet.findShared({ 0,1,2 });
            REQUIRE(res.get() == &i012);
            CHECK(res->isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto res = set.findShared({ 5,0,5 });
            REQUIRE(res.get() == &i505);
            CHECK_FALSE(res->isCalledAsConst());
        }

        SECTION("// invalid") {
            auto const res = set.findShared({ 0,0,0 });
            CHECK(res == nullptr);
        }
    }

    SECTION(".insert()") {
        auto& res = set.insert(std::make_shared<Item>(Point{ 3,2,1 }, 321));
        CHECK(cSet.size() == 3);
        CHECK(&cSet.at({3,2,1}) == &res);
    }

    SECTION(".size()") {
        CHECK(cSet.size() == 2);
    }
}
