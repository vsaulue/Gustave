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

#include <gustave/units/stdStrict/lib/concepts.hpp>
#include <gustave/utils/SizedString.hpp>

namespace gustave::units::stdStrict::lib {
    template<utils::SizedString symbol_>
    struct BasicUnitIdentifier {
        using Char = typename decltype(symbol_)::Char;

        [[nodiscard]]
        static constexpr auto const& symbol() {
            return symbol_;
        };

        [[nodiscard]]
        constexpr BasicUnitIdentifier() = default;

        template<utils::SizedString rhsSymbol>
        [[nodiscard]]
        constexpr bool operator==(BasicUnitIdentifier<rhsSymbol> const&) const {
            return symbol_ == rhsSymbol;
        }

        static_assert(std::same_as<Char, char>, "Only char strings are supported");
    };
}