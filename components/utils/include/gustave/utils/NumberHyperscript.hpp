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
#include <concepts>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

namespace Gustave::Utils {
    struct HyperscriptText {
        static constexpr std::array<std::string_view, 10> digitToText = {
            "\u2070","\u00B9","\u00B2","\u00B3","\u2074","\u2075","\u2076","\u2077","\u2078","\u2079",
        };

        static constexpr std::string_view minusText = "\u207b";

        [[nodiscard]]
        static constexpr std::size_t maxSize() {
            std::size_t result = 0;
            for (const std::string_view& view : digitToText) {
                if (view.size() > result) {
                    result = view.size();
                }
            }
            return result;
        }
    };

    template<std::integral Number>
    class NumberHyperscript {
    public:
        using Unsigned = std::make_unsigned_t<Number>;
        using Char = char;

        using const_iterator = const Char*;

        [[nodiscard]]
        constexpr NumberHyperscript(const Number value)
            : length{0}
            , data_{0} // necessary for constexpr
        {
            Char* it = end();
            auto prependView = [&it](const std::string_view& view) {
                it -= view.size();
                std::copy_n(view.data(), view.size(), it);
            };
            if (value == 0) {
                prependView(HyperscriptText::digitToText[0]);
            } else {
                Unsigned rem = abs(value);
                while (rem > 0) {
                    prependView(HyperscriptText::digitToText[rem % base]);
                    rem /= base;
                }
                if constexpr (std::is_signed_v<Number>) {
                    if (value < 0) {
                        prependView(HyperscriptText::minusText);
                    }
                }
            }
            length = std::distance(it, end());
        }

        static constexpr Unsigned base = 10;

        [[nodiscard]]
        static constexpr std::size_t maxDigitsCount() {
            return std::numeric_limits<Number>::digits10 + 1;
        }

        [[nodiscard]]
        static constexpr std::size_t maxSize() {
            std::size_t result = maxDigitsCount() * HyperscriptText::maxSize();
            if (std::is_signed_v<Number>) {
                result += HyperscriptText::minusText.size();
            }
            return result;
        }

        [[nodiscard]]
        constexpr const_iterator begin() const {
            return end() - length;
        }

        [[nodiscard]]
        constexpr const_iterator end() const {
            return data_ + maxSize();
        }

        [[nodiscard]]
        constexpr std::size_t size() const {
            return length;
        }

        [[nodiscard]]
        constexpr std::string_view view() const {
            return { begin(), length };
        }

        [[nodiscard]]
        constexpr operator std::string_view() const {
            return view();
        }
    private:
        std::size_t length;
        Char data_[maxSize()];

        [[nodiscard]]
        constexpr Char* end() {
            return data_ + maxSize();
        }

        [[nodiscard]]
        static constexpr Unsigned abs(Number value) {
            if constexpr (std::is_unsigned_v<Number>) {
                return value;
            } else {
                if (value >= 0) {
                    return static_cast<Unsigned>(value);
                } else if (value == std::numeric_limits<Number>::min()) {
                    return 1 + static_cast<Unsigned>(-(value + 1));
                } else {
                    return static_cast<Unsigned>(-value);
                }
            }
        }
    };

    template<std::integral Number>
    NumberHyperscript(Number) -> NumberHyperscript<Number>;
}
