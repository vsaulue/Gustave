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

#include <type_traits>

#include <gustave/units/stdStrict/lib/concepts.hpp>
#include <gustave/units/stdStrict/lib/BasicUnitIdentifier.hpp>
#include <gustave/units/stdStrict/lib/Exponent.hpp>

namespace gustave::units::stdStrict::lib {
    template<cBasicUnitIdentifier auto basicUnit_, cExponent auto exponent_>
    struct UnitTerm {
        using Char = decltype(basicUnit_)::Char;

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
        constexpr cUnitTerm auto operator-() const {
            return UnitTerm<basicUnit(), -exponent()>{};
        }

        [[nodiscard]]
        constexpr cUnitTerm auto operator+(cUnitTerm auto rhs) const {
            static_assert(basicUnit_ == rhs.basicUnit(), "Only terms of the same basic unit can be added.");
            return UnitTerm<basicUnit_, exponent_ + rhs.exponent()>{};
        }

        [[nodiscard]]
        constexpr cUnitTerm auto operator-(cUnitTerm auto rhs) const {
            static_assert(basicUnit_ == rhs.basicUnit(), "Only terms of the same basic unit can be substracted.");
            return UnitTerm<basicUnit_, exponent_ - rhs.exponent()>{};
        }

        [[nodiscard]]
        constexpr cUnitTerm auto operator*(cExponent auto rhs) const {
            return UnitTerm<basicUnit_, exponent_ * rhs>{};
        }

        [[nodiscard]]
        constexpr auto operator<=>(cUnitTerm auto rhs) const {
            using RhsChar = decltype(rhs)::Char;
            static_assert(std::is_same_v<Char, RhsChar>, "Only terms using the same Char can be compared.");
            return basicUnit_.symbol() <=> rhs.basicUnit().symbol();
        }
    };
}
