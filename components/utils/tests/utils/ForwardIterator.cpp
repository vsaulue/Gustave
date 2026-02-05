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
#include <cassert>
#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <gustave/utils/ForwardIterator.hpp>
#include <Point.hpp>

namespace utils = gustave::utils;

namespace {
    class Steps {
    private:
        class ValueEnumerator {
        public:
            [[nodiscard]]
            ValueEnumerator()
                : container_{ nullptr }
                , value_{ 0 }
            {}

            [[nodiscard]]
            explicit ValueEnumerator(Steps const& container)
                : container_{ &container }
                , value_{ container.start_ }
            {}

            [[nodiscard]]
            Coord operator*() const {
                assert(value_ <= container_->last_);
                return value_;
            }

            void operator++() {
                assert(value_ <= container_->last_);
                value_ += container_->step_;
            }

            [[nodiscard]]
            bool isEnd() const {
                return value_ > container_->last_;
            }

            [[nodiscard]]
            bool operator==(ValueEnumerator const&) const = default;
        private:
            Steps const* container_;
            int value_;
        };
    public:
        using Iterator = utils::ForwardIterator<ValueEnumerator>;

        [[nodiscard]]
        explicit Steps(int start, int last, int step)
            : start_{ start }
            , last_{ last }
            , step_{ step }
        {
            assert(step_ > 1);
        }

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }
    private:
        int start_;
        int last_;
        int step_;
    };

    class Repeat {
    private:
        class RefEnumerator {
        public:
            [[nodiscard]]
            RefEnumerator()
                : count_{ 0 }
                , value_{ 0 }
            {}

            [[nodiscard]]
            explicit RefEnumerator(Repeat const& container)
                : count_{ container.count_ }
                , value_{ container.value_ }
            {}

            [[nodiscard]]
            Coord& operator*() const {
                assert(count_ > 0);
                return value_;
            }

            void operator++() {
                assert(count_ > 0);
                --count_;
            }

            [[nodiscard]]
            bool isEnd() const {
                return count_ == 0;
            }

            [[nodiscard]]
            bool operator==(RefEnumerator const&) const = default;
        private:
            unsigned count_;
            mutable Coord value_;
        };
    public:
        using Iterator = utils::ForwardIterator<RefEnumerator>;

        [[nodiscard]]
        explicit Repeat(unsigned count, int value)
            : count_{ count }
            , value_{ value }
        {}

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }
    private:
        unsigned count_;
        int value_;
    };
}

static_assert(std::ranges::forward_range<Steps>);
static_assert(std::ranges::forward_range<Repeat>);

TEST_CASE("utils::ForwardIterator") {
    Steps const steps{ 1, 8, 3 };
    Repeat const repeat{ 2, 5 };
    auto stepsIt = steps.begin();
    auto repeatIt = repeat.begin();

    SECTION("::operator*()") {
        SECTION("// value enumerator") {
            CHECK(*stepsIt == Coord{ 1 });
        }

        SECTION("// reference enumerator") {
            *repeatIt = Coord{ 5 };
            CHECK(repeatIt->value == 5);
        }
    }

    SECTION("::operator->()") {
        CHECK(repeatIt->value == 5);
    }

    SECTION("::operator++()") {
        ++stepsIt;
        auto v = *stepsIt;
        CHECK(v.value == 4);
    }

    SECTION("::operator++(int)") {
        auto it2 = stepsIt++;
        CHECK(*stepsIt == Coord{ 4 });
        CHECK(*it2 == Coord{ 1 });
    }

    SECTION("== std::default_sentinel_t") {
        REQUIRE(stepsIt != steps.end());
        ++stepsIt;
        REQUIRE(stepsIt != steps.end());
        ++stepsIt;
        REQUIRE(stepsIt != steps.end());
        ++stepsIt;
        REQUIRE(stepsIt == steps.end());
    }

    SECTION("== ForwardIterator") {
        auto it2 = stepsIt;
        ++stepsIt;
        CHECK(stepsIt != it2);
        ++it2;
        CHECK(stepsIt == it2);
    }

    SECTION("// for loop") {
        SECTION("// value enumerator") {
            auto expected = std::array{ Coord{1}, Coord{4}, Coord{7} };
            CHECK_THAT(steps, Catch::Matchers::RangeEquals(expected));
        }

        SECTION("// reference enumerator") {
            auto expected = std::array{ Coord{5}, Coord{5} };
            CHECK_THAT(repeat, Catch::Matchers::RangeEquals(expected));
        }
    }
}
