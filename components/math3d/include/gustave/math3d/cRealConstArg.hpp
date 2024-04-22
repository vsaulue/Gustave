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

#pragma once

#include <concepts>

#include <gustave/cfg/cRealOf.hpp>
#include <gustave/cfg/cRealTraits.hpp>

namespace gustave::math3d {
    namespace detail {
        template<cfg::cRealTraits Traits>
        [[nodiscard]]
        constexpr cfg::cRealOf<Traits{}> auto asRealConstArg(Traits, std::floating_point auto value) {
            constexpr auto one = Traits::units().one;
            using Real = typename Traits::template Type<one, decltype(value)>;
            return Real{ value };
        }

        template<cfg::cRealTraits Traits, cfg::cRealOf<Traits{}> Real>
        [[nodiscard]]
        constexpr Real asRealConstArg(Traits, Real const value) {
            return value;
        }
    }

    template<typename T, auto traits>
    concept cRealConstArg = requires (T value) {
        requires cfg::cRealTraits<decltype(traits)>;
        { detail::asRealConstArg(traits, value) } -> cfg::cRealOf<traits>;
    };
}
