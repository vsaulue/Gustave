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

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <cstdint>
#include <ostream>
#include <string_view>

#include <gustave/meta/Meta.hpp>
#include <gustave/utils/cChar.hpp>
#include <gustave/utils/SizedStringView.hpp>

namespace Gustave::Utils {
    template<cChar _Char, std::size_t length>
    struct SizedString {
    public:
        using Char = _Char;
        using Size = std::size_t;
        using Data = std::array<Char, length>;

        using const_iterator = typename Data::const_iterator;
        using iterator = typename Data::iterator;

        using string_view = std::basic_string_view<Char>;

        [[nodiscard]]
        constexpr SizedString()
            requires (length == 0)
        :
            data{} // necessary for MSVC
        {

        }

        [[nodiscard]]
        explicit constexpr SizedString(SizedStringView<Char, length> val) {
            if constexpr (size() == 0) {
                data = {}; // necessary for MSVC
            } else {
                std::copy_n(val.begin(), size(), begin());
            }
        }

        [[nodiscard]] 
        constexpr SizedString(const Char (&val)[length+1]) {
            if constexpr (size() == 0) {
                data = {}; // necessary for MSVC
            } else {
                std::copy_n(val, size(), begin());
            }
        }

        template<Size... argLen>
        [[nodiscard]]
        constexpr SizedString(SizedStringView<Char, argLen>... subStrings)
            requires (sizeof...(argLen) > 1)
        {
            static_assert((subStrings.size() + ...) == length, "Invalid concatenation: size mismatch.");
            auto it = begin();
            ([&it,subStrings] {
                it = std::copy_n(subStrings.begin(), subStrings.size(), it);
            }(), ...);
        }

        [[nodiscard]] 
        static constexpr Size size() {
            return length;
        }

        [[nodiscard]] 
        constexpr iterator begin() {
            return data.begin();
        }

        [[nodiscard]]
        constexpr const_iterator begin() const {
            return data.begin();
        }

        [[nodiscard]]
        constexpr iterator end() {
            return data.end();
        }

        [[nodiscard]]
        constexpr const_iterator end() const {
            return data.end();
        }

        [[nodiscard]]
        constexpr operator string_view() const {
            return view();
        }

        [[nodiscard]]
        constexpr string_view view() const {
            if constexpr (size() == 0) {
                return { nullptr, size() };
            } else {
                return { &data.front(), size() };
            }
        }

        [[nodiscard]]
        constexpr SizedStringView<Char, length> sizedView() const {
            if constexpr (size() == 0) {
                return { nullptr };
            } else {
                return { &data.front() };
            }
        }

        Data data;
    };

    template<typename T>
    concept cSizedString = std::same_as<T,SizedString<typename T::Char, T::size()>>;

    template<cChar Char, std::size_t length>
    SizedString(const Char(&val)[length]) -> SizedString<Char, length - 1>;

    template<cChar Char, std::size_t length>
    explicit SizedString(SizedStringView<Char, length>) -> SizedString<Char, length>;

    template<cChar Char, std::size_t... argLen>
        requires (sizeof...(argLen) > 1)
    SizedString(SizedStringView<Char, argLen>...) -> SizedString<Char, (argLen + ...)>;

    [[nodiscard]]
    constexpr std::strong_ordering operator<=>(cSizedString auto const& lhs, cSizedString auto const& rhs) {
        using LhsChar = typename decltype(Meta::value(lhs))::Char;
        using RhsChar = typename decltype(Meta::value(rhs))::Char;
        static_assert(std::is_same_v<LhsChar,RhsChar>,"Invalid comparison: incompatible character types");
        return lhs.view() <=> rhs.view();
    }

    template<cChar Char, std::size_t length>
    std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& stream, SizedString<Char,length> const& string) {
        using Str = decltype(Meta::value(string));
        if constexpr (Str::size() > 0) {
            stream.write(&string.data.front(), string.size());
        }
        return stream;
    }
}
