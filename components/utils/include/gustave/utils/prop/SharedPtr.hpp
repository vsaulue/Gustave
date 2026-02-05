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

#pragma once

#include <concepts>
#include <memory>
#include <utility>

#include <gustave/meta/Meta.hpp>

namespace gustave::utils::prop {
    template<meta::cNotCvRef T>
    class SharedPtr;

    template<typename T>
    concept cSharedPtr = std::same_as<T, SharedPtr<typename T::element_type>>;

    template<meta::cNotCvRef T>
    class SharedPtr {
    private:
        std::shared_ptr<T> value_;
    public:
        using element_type = T;

        [[nodiscard]]
        constexpr SharedPtr() = default;

        [[nodiscard]]
        constexpr SharedPtr(std::nullptr_t null)
            : value_{ null }
        {}

        [[nodiscard]]
        SharedPtr(std::shared_ptr<T> value)
            : value_{ std::move(value) }
        {}

        [[nodiscard]]
        SharedPtr(SharedPtr&) = default;

        [[nodiscard]]
        SharedPtr(SharedPtr&&) = default;

        SharedPtr& operator=(std::nullptr_t null) {
            value_ = null;
            return *this;
        }

        SharedPtr& operator=(std::shared_ptr<T> value) {
            value_ = std::move(value);
            return *this;
        }

        SharedPtr& operator=(SharedPtr&) = default;

        SharedPtr& operator=(SharedPtr&&) = default;

        [[nodiscard]]
        T* operator->() {
            return value_.get();
        }

        [[nodiscard]]
        T const* operator->() const {
            return value_.get();
        }

        [[nodiscard]]
        T& operator*() {
            return *value_;
        }

        [[nodiscard]]
        T const& operator*() const {
            return *value_;
        }

        [[nodiscard]]
        bool operator==(std::nullptr_t null) const {
            return value_ == null;
        }

        [[nodiscard]]
        operator bool() const {
            return bool{ value_ };
        }

        [[nodiscard]]
        operator std::shared_ptr<T const>() const& {
            return value_;
        }

        [[nodiscard]]
        operator std::shared_ptr<T const>()&& {
            return std::move(value_);
        }

        [[nodiscard]]
        std::shared_ptr<T const> asImmutable() const& {
            return value_;
        }

        [[nodiscard]]
        std::shared_ptr<T const> asImmutable() && {
            return std::move(value_);
        }

        [[nodiscard]]
        T* get() {
            return value_.get();
        }

        [[nodiscard]]
        T const* get() const {
            return value_.get();
        }

        [[nodiscard]]
        std::shared_ptr<T> const& unprop() {
            return value_;
        }

        [[nodiscard]]
        bool operator==(cSharedPtr auto const& rhs) const
            requires requires { get() == rhs.get(); }
        {
            return value_.get() == rhs.get();
        }

        template<typename U>
        [[nodiscard]]
        bool operator==(std::shared_ptr<U> const& rhs) const
            requires requires { value_ == rhs; }
        {
            return value_ == rhs;
        }
    };
}
