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

#include <gustave/utils/IndexGenerator.hpp>

namespace utils = gustave::utils;

using IndexGenerator = utils::IndexGenerator<std::uint32_t>;

TEST_CASE("utils::IndexGenerator") {
    auto gen = IndexGenerator{};

    SECTION("// constructor") {
        SECTION("- default") {
            CHECK(gen.readNextIndex() == 1);
        }

        SECTION("- custom first") {
            auto gen123 = IndexGenerator{ 123 };
            CHECK(gen123.readNextIndex() == 123);
        }

        SECTION("- invalid") {
            CHECK_THROWS_AS(IndexGenerator{ 0 }, std::invalid_argument);
        }
    }

    SECTION("operator()()") {
        SECTION("// valid") {
            CHECK(gen() == 1);
            CHECK(gen() == 2);
            CHECK(gen() == 3);
        }

        SECTION("// invalid") {
            static constexpr auto MAX = std::numeric_limits<IndexGenerator::IndexType>::max();
            auto genMax = IndexGenerator{ MAX };
            CHECK(genMax() == MAX);
            CHECK_THROWS_AS(genMax(), std::overflow_error);
        }
    }

    SECTION("readNextIndex()") {
        CHECK(gen.readNextIndex() == 1);
        CHECK(gen.readNextIndex() == 1);
        CHECK(gen() == 1);
        CHECK(gen() == 2);
        CHECK(gen.readNextIndex() == 3);
        CHECK(gen.readNextIndex() == 3);
        CHECK(gen() == 3);
    }
}
