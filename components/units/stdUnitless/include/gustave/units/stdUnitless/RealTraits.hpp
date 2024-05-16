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

#include <cmath>
#include <type_traits>

#include <gustave/meta/Meta.hpp>
#include <gustave/units/stdUnitless/lib/Real.hpp>
#include <gustave/units/stdUnitless/UnitSystem.hpp>

namespace gustave::units::stdUnitless {
    struct RealTraits {
        [[nodiscard]]
        static constexpr UnitSystem units() {
            return {};
        }

        template<lib::Unit, cfg::cRealRep Rep>
        using Type = lib::Real<Rep>;

        [[nodiscard]]
        static constexpr bool isUnit(auto unit) {
            return std::is_same_v<decltype(unit), lib::Unit>;
        }

        [[nodiscard]]
        constexpr RealTraits() = default;

        [[nodiscard]]
        static auto sqrt(lib::cReal auto const arg) -> decltype(meta::value(arg)) {
            return lib::Real{ std::sqrt(arg.value()) };
        }

        [[nodiscard]]
        static constexpr auto abs(lib::cReal auto const arg) -> decltype(meta::value(arg)) {
            return std::abs(arg.value());
        }

        [[nodiscard]]
        static constexpr auto max(lib::cReal auto const lhs, lib::cReal auto const rhs) -> decltype(lhs + rhs) {
            if (lhs > rhs) {
                return lhs;
            } else {
                return rhs;
            }
        }

        [[nodiscard]]
        static constexpr auto min(lib::cReal auto const lhs, lib::cReal auto const rhs) -> decltype(lhs + rhs) {
            if (lhs < rhs) {
                return lhs;
            } else {
                return rhs;
            }
        }

        [[nodiscard]]
        static bool signBit(lib::cReal auto const real) {
            return std::signbit(real.value());
        }

        [[nodiscard]]
        constexpr bool operator==(RealTraits const&) const = default;
    };
}
