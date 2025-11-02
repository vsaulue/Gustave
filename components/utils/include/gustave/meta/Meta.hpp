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

#include <concepts>
#include <type_traits>

namespace gustave::meta {
    namespace detail {
        template<typename T>
        constexpr bool alwaysFalse = false;
    }

    /**
    * Metaprograming tool to remove cvref qualifiers in unevaluated context.
    */
    template<typename T>
    consteval std::remove_cvref_t<T> value(T&&) {
        static_assert(detail::alwaysFalse<T>, "Meta::value() cannot be used in an evaluated context.");
    }

    template<typename T, typename Desired>
    concept cCvRefOf = std::same_as<Desired, std::remove_cvref_t<T>>;

    template<typename T>
    concept cNotCvRef = std::same_as<T, std::remove_cvref_t<T>>;

    template<bool condition, typename T>
    using MutableIf = std::conditional_t<condition, T, T const>;

    /**
    * Simple type wrapper.
    *
    * It can be used to return any type (with possibly cvref qualifiers) from a consteval function.
    */
    template<typename T>
    struct TypeWrapper {
        using Type = T;
    };

    template<typename T>
    concept cTypeWrapper = std::is_same_v<T, TypeWrapper<typename T::Type>>;
}
