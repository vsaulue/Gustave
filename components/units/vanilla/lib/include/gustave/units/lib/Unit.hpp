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

#include <ostream>
#include <type_traits>

#include <gustave/utils/SizedString.hpp>

#include "Exponent.hpp"
#include "UnitIdentifier.hpp"

namespace gustave::units::lib {
    template<utils::SizedString symbol_, cUnitIdentifier auto unitId_>
    class Unit;
    
    template<typename T>
    concept cUnit = std::same_as<T, Unit<T::symbol(), T::unitId()>>;

    // `cUnitidentifier auto unitId_` instead of `UnitIdentifier unitId_`, because of MSVC bug.
    // see https://developercommunity.visualstudio.com/t/CTAD-rule-on-default-constructible-type/10279590?sort=newest.
    template<utils::SizedString symbol_, cUnitIdentifier auto unitId_>
    class Unit {
    public:
        using UnitId = decltype(unitId_);
        using Symbol = decltype(symbol_);

        [[nodiscard]]
        constexpr Unit() = default;

        [[nodiscard]]
        constexpr Unit(UnitId) {

        }

        [[nodiscard]]
        static constexpr Symbol symbol() {
            return symbol_;
        }

        [[nodiscard]]
        static constexpr UnitId unitId() {
            return unitId_;
        }

        [[nodiscard]]
        static constexpr bool isOne() {
            return unitId().isOne();
        }

        [[nodiscard]]
        static constexpr bool isTrivialOne() {
            return isOne() && (symbol_.size() == 0);
        }

        [[nodiscard]]
        static constexpr cUnit auto inverse() {
            constexpr cUnitIdentifier auto resId = unitId_.inverse();
            return Unit<resId.toString(), resId>{};
        }

        [[nodiscard]]
        static constexpr cUnit auto pow(cExponent auto exp) {
            constexpr cUnitIdentifier auto resId = unitId_.pow(exp);
            return Unit<resId.toString(), resId>{};
        }

        template<utils::SizedString argSymbol, UnitIdentifier argUnitId>
        [[nodiscard]]
        static constexpr bool isAssignableFrom(Unit<argSymbol, argUnitId> other) {
            return other.unitId() == unitId();
        }

        [[nodiscard]]
        static constexpr bool isAssignableFrom(cUnitIdentifier auto otherId) {
            return otherId == unitId();
        }
    };

    template<cUnitIdentifier UnitId>
    Unit(UnitId) -> Unit<UnitId::toString(), UnitId{}>;

    inline constexpr Unit<"", UnitIdentifier<>{}> one{};

    [[nodiscard]]
    constexpr bool operator==(cUnit auto lhs, cUnit auto rhs) {
        return std::is_same_v<decltype(lhs), decltype(rhs)>;
    }

    [[nodiscard]]
    constexpr auto operator*(cUnit auto lhs, cUnit auto rhs) {
        return Unit(lhs.unitId() * rhs.unitId());
    }

    [[nodiscard]]
    constexpr auto operator/(cUnit auto lhs, cUnit auto rhs) {
        return Unit(lhs.unitId() / rhs.unitId());
    }

    std::ostream& operator<<(std::ostream& stream, cUnit auto unit) {
        return stream << unit.symbol();
    }

}
