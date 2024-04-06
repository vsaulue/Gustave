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

#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include <gustave/utils/ForwardIterator.hpp>
#include <Point.hpp>

namespace utils = gustave::utils;

namespace {
    class Steps {
    private:
        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : container_{ nullptr }
                , value_{ 0 }
            {}

            [[nodiscard]]
            explicit Enumerator(Steps const& container)
                : container_{ &container }
                , value_{ container.start_ }
            {}

            [[nodiscard]]
            Coord const& operator*() const {
                return value_;
            }

            void operator++() {
                value_.value += container_->step_;
            }

            [[nodiscard]]
            bool isEnd() const {
                return value_.value > container_->end_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const&) const = default;
        private:
            Steps const* container_;
            Coord value_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Steps(int start, int end, int step)
            : start_{ start }
            , end_{ end }
            , step_{ step }
        {
            assert(step_ > 1);
        }

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr utils::EndIterator end() const {
            return {};
        }
    private:
        int start_;
        int end_;
        int step_;
    };
}

static_assert(std::ranges::forward_range<Steps>);

TEST_CASE("utils::ForwardIterator") {
    Steps const steps{ 1, 8, 3 };
    Steps::Iterator it = steps.begin();

    SECTION("::operator*()") {
        CHECK(*it == Coord{ 1 });
    }

    SECTION("::operator->()") {
        CHECK(it->value == 1);
    }

    SECTION("::operator++()") {
        ++it;
        CHECK(it->value == 4);
    }

    SECTION("::operator++(int)") {
        auto it2 = it++;
        CHECK(*it == Coord{ 4 });
        CHECK(*it2 == Coord{ 1 });
    }

    SECTION("== EndIterator") {
        REQUIRE(it != steps.end());
        ++it;
        REQUIRE(it != steps.end());
        ++it;
        REQUIRE(it != steps.end());
        ++it;
        REQUIRE(it == steps.end());
    }

    SECTION("== ForwardIterator") {
        auto it2 = it;
        ++it;
        CHECK(it != it2);
        ++it2;
        CHECK(it == it2);
    }

    SECTION("// for loop") {
        std::vector<Coord> expected{ Coord{1}, Coord{4}, Coord{7} };
        CHECK_THAT(steps, Catch::Matchers::RangeEquals(expected));
    }
}
