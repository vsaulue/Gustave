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

#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <gustave/utils/PointerHash.hpp>

namespace utils = gustave::utils;

TEST_CASE("utils::PointerHash") {
    SECTION("::Set<unique_ptr<*>>") {
        utils::PointerHash::Set<std::unique_ptr<int>> set;
        auto unique = std::make_unique<int>(8);
        int* const raw = unique.get();
        set.emplace(std::move(unique));

        CHECK(set.contains(raw));
        CHECK_FALSE(set.contains(nullptr));

        auto otherUnique = std::make_unique<int>(8);
        CHECK_FALSE(set.contains(otherUnique));
    }

    SECTION("::Set<shared_ptr<*>>") {
        utils::PointerHash::Set<std::shared_ptr<int>> set;
        auto shared = std::make_shared<int>(5);
        set.insert(shared);

        CHECK(set.contains(shared));
        CHECK(set.contains(shared.get()));
        CHECK_FALSE(set.contains(nullptr));

        auto unique = std::make_unique<int>(5);
        CHECK_FALSE(set.contains(unique));
    }
}
