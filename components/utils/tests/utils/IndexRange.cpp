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

#include <catch2/catch_test_macros.hpp>

#include <gustave/utils/IndexRange.hpp>

namespace utils = gustave::utils;

TEST_CASE("utils::IndexRange") {
    auto ir = utils::IndexRange<std::uint32_t>{ 2,3 };

    SECTION(".setSize()") {
        ir.setSize(7);
        CHECK(ir.size() == 7);
    }

    SECTION(".setStart()") {
        ir.setStart(10);
        CHECK(ir.start() == 10);
    }

    SECTION(".size()") {
        CHECK(ir.size() == 3);
    }

    SECTION(".start()") {
        CHECK(ir.start() == 2);
    }

    SECTION(".subSpanOf(std::span)") {
        auto const vec = std::vector<int>{ -1, -2, -3, -4, -5, -6 };
        auto const span = std::span<int const>{ vec };
        auto const res = ir.subSpanOf(span);
        CHECK(res.data() == &span[2]);
        CHECK(res.size() == 3);
    }

    SECTION(".subSpanOf(std::vector)") {
        auto const vec = std::vector<int>{ -1,-2,-3,-4,-5,-6 };
        std::span<int const> const res = ir.subSpanOf(vec);
        CHECK(res.data() == &vec[2]);
        CHECK(res.size() == 3);
    }
}
