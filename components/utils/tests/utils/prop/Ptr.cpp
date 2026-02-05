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

#include <concepts>

#include <catch2/catch_test_macros.hpp>

#include <gustave/utils/prop/Ptr.hpp>
#include <gustave/testing/ConstDetector.hpp>
#include <gustave/testing/cPropPtr.hpp>

template<typename T>
using Ptr = gustave::utils::prop::Ptr<T>;

using ConstDetector = gustave::testing::ConstDetector<int>;

static_assert(gustave::testing::cPropPtr<Ptr<int>>);

TEST_CASE("utils::prop::Ptr") {
    ConstDetector d1{ 1 };
    ConstDetector d2{ 2 };
    Ptr<ConstDetector> const cPtr = &d1;
    Ptr<ConstDetector> ptr = &d1;

    SECTION("operator=(Ptr<T>&)") {
        Ptr<ConstDetector> ptr2 = nullptr;
        ptr2 = ptr;
        CHECK(ptr2->tag == 1);
    }

    SECTION("operator=(rawptr*)") {
        auto&res = (ptr = &d2);
        CHECK(ptr->tag == 2);
        CHECK(&res == &ptr);
    }

    SECTION("operator*()") {
        SECTION("// mutable") {
            CHECK((*ptr).tag == 1);
            CHECK_FALSE((*ptr).isCalledAsConst());
        }

        SECTION("// const") {
            CHECK((*ptr).tag == 1);
            CHECK((*cPtr).isCalledAsConst());
        }
    }

    SECTION("operator->()") {
        SECTION("// mutable") {
            CHECK(ptr->tag == 1);
            CHECK_FALSE(ptr->isCalledAsConst());
        }

        SECTION("// const") {
            CHECK(ptr->tag == 1);
            CHECK(cPtr->isCalledAsConst());
        }
    }

    SECTION("operator T const*() const") {
        ConstDetector const* rawPtr = cPtr;
        CHECK(rawPtr == &d1);
        CHECK(rawPtr->isCalledAsConst());
    }

    SECTION("operator T*()") {
        ConstDetector* rawPtr = ptr;
        CHECK(rawPtr == &d1);
        CHECK_FALSE(rawPtr->isCalledAsConst());
    }

    SECTION("asImmutable()") {
        auto res = cPtr.asImmutable();
        CHECK(res == &d1);
        CHECK(res->isCalledAsConst());
    }
}
