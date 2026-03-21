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

#include <cassert>
#include <cstdint>
#include <string>

#include <tinyxml2.h>

#include <gustave/examples/jsonGustave/CStringView.hpp>

namespace gustave::examples::jsonGustave::xmlWriter::detail {
    class XmlWriterData {
    public:
        using CStringView = jsonGustave::CStringView;
        using Depth = std::int64_t;
        using Printer = tinyxml2::XMLPrinter;

        [[nodiscard]]
        explicit XmlWriterData(std::FILE* output, bool compactMode)
            : printer_{ output, compactMode }
            , compactMode_{ compactMode }
            , depth_{ -1 }
        {}

        [[nodiscard]]
        Depth depth() const {
            return depth_;
        }

        void attr(CStringView name, auto const& value)
            requires requires (Printer& p) { p.PushAttribute(name.cStr(), value); }
        {
            assert(hasOpenedElements());
            printer_.PushAttribute(name.cStr(), value);
        }

        void attr(CStringView name, CStringView value) {
            assert(hasOpenedElements());
            printer_.PushAttribute(name.cStr(), value.cStr());
        }

        void attr(CStringView name, std::string const& value) {
            assert(hasOpenedElements());
            printer_.PushAttribute(name.cStr(), value.c_str());
        }

        Depth newElement(CStringView name) {
            printer_.OpenElement(name.cStr(), compactMode_);
            ++depth_;
            return depth_;
        }

        void closeElement() {
            assert(hasOpenedElements());
            printer_.CloseElement(compactMode_);
            --depth_;
        }

        [[nodiscard]]
        bool hasOpenedElements() const {
            return depth_ >= 0;
        }

        void text(CStringView content) {
            assert(hasOpenedElements());
            printer_.PushText(content.cStr());
        }
    private:
        Printer printer_;
        bool compactMode_;
        Depth depth_;
    };
}
