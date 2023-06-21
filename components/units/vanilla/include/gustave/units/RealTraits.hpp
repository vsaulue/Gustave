/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <type_traits>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/units/lib/Math.hpp>
#include <gustave/units/lib/Real.hpp>
#include <gustave/units/lib/Unit.hpp>
#include <gustave/units/UnitSystem.hpp>

namespace Gustave::Units {
    template<Cfg::cRealRep Rep>
    struct RealTraits {
        [[nodiscard]]
        static constexpr UnitSystem units() {
            return {};
        }

        template<Lib::cUnit auto unit>
        using Type = Lib::Real<unit, Rep>;

        [[nodiscard]]
        static constexpr bool isUnit(auto unit) {
            return Lib::cUnit<decltype(unit)>;
        }

        [[nodiscard]]
        constexpr RealTraits() = default;

        [[nodiscard]]
        static Lib::cReal auto sqrt(Lib::cReal auto const arg) {
            return Lib::sqrt(arg);
        }

        template<Lib::cReal Arg>
        [[nodiscard]]
        static constexpr Arg abs(Arg const arg) {
            return (arg.value() > 0.f) ? arg : -arg;
        }

        template<Lib::cReal Arg>
        [[nodiscard]]
        static constexpr Arg max(Arg const v1, Arg const v2) {
            return (v1 > v2) ? v1 : v2;
        }

        template<Lib::cReal Arg>
        [[nodiscard]]
        static constexpr Arg min(Arg const v1, Arg const v2) {
            return (v1 < v2) ? v1 : v2;
        }

        template<Cfg::cRealRep ArgRep>
        [[nodiscard]]
        constexpr bool operator==(RealTraits<ArgRep> const&) const {
            return std::is_same_v<Rep, ArgRep>;
        }
    };
}
