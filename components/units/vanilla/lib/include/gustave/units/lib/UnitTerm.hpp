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

#include <type_traits>

#include "BasicUnitIdentifier.hpp"
#include "Exponent.hpp"

namespace Gustave::Units::Lib {
    template<cBasicUnitIdentifier auto basicUnit_, Exponent exponent_>
    struct UnitTerm {
        using Char = typename decltype(basicUnit_)::Char;

        [[nodiscard]]
        constexpr UnitTerm() = default;

        [[nodiscard]]
        static constexpr auto exponent() {
            return exponent_;
        }

        [[nodiscard]]
        static constexpr auto basicUnit() {
            return basicUnit_;
        }

        [[nodiscard]]
        constexpr auto operator-() const {
            return UnitTerm<basicUnit(), -exponent()>{};
        }
    };

    template<typename T>
    concept cUnitTerm = std::same_as<T, UnitTerm<T::basicUnit(), T::exponent()>>;

    [[nodiscard]]
    constexpr auto operator+(cUnitTerm auto lhs, cUnitTerm auto rhs) {
        static_assert(lhs.basicUnit() == rhs.basicUnit(), "Only terms of the same basic unit can be added.");
        return UnitTerm<lhs.basicUnit(), lhs.exponent() + rhs.exponent()>{};
    }

    [[nodiscard]]
    constexpr auto operator-(cUnitTerm auto lhs, cUnitTerm auto rhs) {
        static_assert(lhs.basicUnit() == rhs.basicUnit(), "Only terms of the same basic unit can be substracted.");
        return UnitTerm<lhs.basicUnit(), lhs.exponent() - rhs.exponent()>{};
    }

    [[nodiscard]]
    constexpr auto operator*(cUnitTerm auto lhs, cExponent auto rhs) {
        return UnitTerm<lhs.basicUnit(), lhs.exponent()* rhs>{};
    }

    [[nodiscard]]
    constexpr auto operator<=>(cUnitTerm auto lhs, cUnitTerm auto rhs) {
        using LhsChar = typename decltype(lhs)::Char;
        using RhsChar = typename decltype(rhs)::Char;
        static_assert(std::is_same_v<LhsChar, RhsChar>, "Only terms using the same Char can be compared.");
        return lhs.basicUnit().symbol() <=> rhs.basicUnit().symbol();
    }
}
