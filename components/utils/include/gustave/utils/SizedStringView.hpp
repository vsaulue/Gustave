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

#include <cstddef>

#include <gustave/utils/cChar.hpp>

namespace gustave::utils {
    template<cChar Char_, std::size_t length>
    class SizedStringView {
    public:
        using Char = Char_;
        using Size = std::size_t;

        using const_iterator = const Char*;

        [[nodiscard]]
        constexpr SizedStringView(const Char* val)
            : data_(val)
        {

        }

        [[nodiscard]]
        constexpr const Char* data() const {
            return data_;
        }

        [[nodiscard]]
        static constexpr std::size_t size() {
            return length;
        }

        [[nodiscard]]
        constexpr const_iterator begin() const {
            return data_;
        }

        [[nodiscard]]
        constexpr const_iterator end() const {
            return data_ + length;
        }
    private:
        const Char* data_;
    };

    template<cChar Char, std::size_t len>
    SizedStringView(const Char(&)[len]) -> SizedStringView<Char, len - 1>;
}
