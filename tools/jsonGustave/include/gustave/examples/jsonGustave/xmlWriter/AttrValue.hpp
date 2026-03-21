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

#include <gustave/examples/jsonGustave/CStringView.hpp>

#include <concepts>
#include <string>
#include <variant>

namespace gustave::examples::jsonGustave::xmlWriter {
    class AttrValue {
    private:
        using CStringView = jsonGustave::CStringView;
        using Variant = std::variant<CStringView, bool, std::int64_t, std::uint64_t, double>;
    public:
        [[nodiscard]]
        AttrValue(auto const& val)
            requires (std::constructible_from<Variant, decltype(val)>)
        : value_{ val }
        {}

        [[nodiscard]]
        AttrValue(std::string const& val)
            : value_{ val.c_str() }
        {}

        [[nodiscard]]
        Variant const& value() const {
            return value_;
        }

        AttrValue& operator=(auto const& val)
            requires requires (Variant& variant) { variant = val; }
        {
            value_ = val;
            return *this;
        }

        AttrValue& operator=(std::string const& val) {
            value_ = val.c_str();
            return *this;
        }

        void visit(auto&& visitor) const {
            std::visit(std::forward<decltype(visitor)>(visitor), value_);
        }
    private:
        Variant value_;
    };
}
