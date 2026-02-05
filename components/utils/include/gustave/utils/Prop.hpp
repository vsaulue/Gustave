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

#include <gustave/meta/Meta.hpp>
#include <gustave/utils/prop/Ptr.hpp>
#include <gustave/utils/prop/SharedPtr.hpp>

#include <memory>
#include <ranges>
#include <type_traits>

namespace gustave::utils {
    template<bool isMut, meta::cNotCvRef T>
    using Prop = std::conditional_t<isMut, T, T const>;

    template<bool isMut, meta::cNotCvRef T>
    using PropIterator = decltype(std::ranges::begin(std::declval<Prop<isMut,T>&>()));

    template<bool isMut, meta::cNotCvRef T>
    using PropPtr = std::conditional_t<isMut, prop::Ptr<T>, T const*>;

    template<bool isMut, meta::cNotCvRef T>
    using PropSharedPtr = std::conditional_t<isMut, prop::SharedPtr<T>, std::shared_ptr<T const>>;
}
