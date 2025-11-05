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

#pragma once

namespace gustave::utils::prop {
    template<typename T>
    class Ptr {
    private:
        static_assert(!std::is_reference_v<T>, "Reference type forbidden.");
        static_assert(!std::is_const_v<T>, "Pointer to const type forbidden. Use 'T const*' instead.");
    public:
        [[nodiscard]]
        constexpr Ptr(T* value)
            : value_{ value }
        {}

        [[nodiscard]]
        constexpr Ptr(Ptr&) = default;

        constexpr Ptr& operator=(Ptr&) = default;

        constexpr Ptr& operator=(T* value) {
            value_ = value;
            return *this;
        }

        [[nodiscard]]
        constexpr operator T* () {
            return value_;
        }

        [[nodiscard]]
        constexpr operator T const* () const {
            return value_;
        }

        [[nodiscard]]
        constexpr T* operator->() {
            return value_;
        }

        [[nodiscard]]
        constexpr T const* operator->() const {
            return value_;
        }

        [[nodiscard]]
        constexpr T& operator*() {
            return *value_;
        }

        [[nodiscard]]
        constexpr T const& operator*() const {
            return *value_;
        }

        [[nodiscard]]
        T const* asImmutable() const {
            return value_;
        }
    private:
        T* value_;
    };

    template<bool isMutable, typename T>
    using PtrMember = std::conditional_t<isMutable, Ptr<T>, T const*>;
}
