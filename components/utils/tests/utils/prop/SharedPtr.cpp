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

#include <memory>

#include <catch2/catch_test_macros.hpp>

#include <gustave/utils/prop/SharedPtr.hpp>
#include <gustave/testing/ConstDetector.hpp>
#include <gustave/testing/cPropPtr.hpp>

template<typename T>
using SharedPtr = gustave::utils::prop::SharedPtr<T>;

using ConstDetector = gustave::testing::ConstDetector<int>;

static_assert(gustave::testing::cPropPtr<SharedPtr<int>>);

TEST_CASE("utils::prop::SharedPtr") {
    auto ptr = SharedPtr<ConstDetector>{ std::make_shared<ConstDetector>(123) };
    auto const cPtr = ptr;

    SECTION("::SharedPtr()") {
        auto const obj = SharedPtr<int>{};
        CHECK(obj.get() == nullptr);
    }

    SECTION("::SharedPtr(nullptr)") {
        auto const obj = SharedPtr<int>{ nullptr };
        CHECK(obj.get() == nullptr);
    }

    SECTION("::SharedPtr(std::shared_ptr)") {
        CHECK(ptr->tag == 123);
    }

    SECTION("operator=(nullptr)") {
        auto& ref = (ptr = nullptr);
        CHECK(&ref == &ptr);
        CHECK(ptr.get() == nullptr);
    }

    SECTION("operator=(std::shared_ptr)") {
        auto& ref = (ptr = std::make_shared<ConstDetector>(456));
        CHECK(&ref == &ptr);
        CHECK(ptr->tag == 456);
        CHECK(cPtr->tag == 123);
    }

    SECTION("operator->()") {
        SECTION("// mutable") {
            CHECK_FALSE(ptr->isCalledAsConst());
            ptr->tag = 456;
            CHECK(cPtr->tag == 456);
        }

        SECTION("// const") {
            CHECK(cPtr->isCalledAsConst());
            CHECK(cPtr->tag == 123);
        }
    }

    SECTION("operator*()") {
        SECTION("// mutable") {
            CHECK_FALSE((*ptr).isCalledAsConst());
        }

        SECTION("// const") {
            CHECK((*cPtr).isCalledAsConst());
        }
    }

    SECTION("operator==(nullptr)") {
        SECTION("// true") {
            CHECK(SharedPtr<int>{} == nullptr);
        }

        SECTION("// false") {
            CHECK_FALSE(cPtr == nullptr);
        }
    }

    SECTION("operator bool()") {
        SECTION("// true") {
            CHECK(cPtr);
        }

        SECTION("// false") {
            CHECK_FALSE(SharedPtr<int>{});
        }
    }

    SECTION("operator std::shared_ptr()") {
        SECTION("const&") {
            std::shared_ptr<ConstDetector const> stdPtr = cPtr;
            CHECK(stdPtr.get() == cPtr.get());
        }

        SECTION("&&") {
            std::shared_ptr<ConstDetector const> stdPtr = std::move(ptr);
            CHECK(stdPtr.get() == cPtr.get());
            CHECK_FALSE(ptr);
        }
    }

    SECTION(".asImmutable()") {
        SECTION("const&") {
            auto const stdPtr = cPtr.asImmutable();
            CHECK(stdPtr.get() == cPtr.get());
        }

        SECTION("&&") {
            auto const stdPtr = std::move(ptr).asImmutable();
            CHECK(stdPtr.get() == cPtr.get());
            CHECK_FALSE(ptr);
        }
    }

    SECTION(".get()") {
        SECTION("const") {
            CHECK(cPtr.get()->isCalledAsConst());
            CHECK(cPtr.get()->tag == 123);
        }

        SECTION("&") {
            CHECK_FALSE(ptr.get()->isCalledAsConst());
            ptr.get()->tag = 222;
            CHECK(cPtr.get()->tag == 222);
        }
    }

    SECTION(".unprop()") {
        auto const& unprop = ptr.unprop();
        CHECK(unprop.get() == ptr.get());
    }

    SECTION(".operator==(std::shared_ptr)") {
        auto const iPtr = std::shared_ptr<ConstDetector const>{ ptr };
        CHECK(iPtr == ptr);
        CHECK(cPtr == iPtr);
        CHECK(ptr != std::shared_ptr<ConstDetector const>{});
    }

    SECTION(".operator==(SharedPtr)") {
        auto const ptr2 = SharedPtr<ConstDetector>{ std::make_shared<ConstDetector>(123) };
        CHECK(cPtr == ptr);
        CHECK(ptr != ptr2);
    }
}
