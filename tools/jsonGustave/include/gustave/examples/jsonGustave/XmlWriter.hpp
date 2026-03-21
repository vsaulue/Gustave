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

#include <cstdio>
#include <format>
#include <memory>
#include <utility>

#include <gustave/examples/jsonGustave/xmlWriter/detail/XmlWriterData.hpp>
#include <gustave/examples/jsonGustave/xmlWriter/Attrs.hpp>
#include <gustave/examples/jsonGustave/xmlWriter/Element.hpp>
#include <gustave/examples/jsonGustave/CStringView.hpp>

namespace gustave::examples::jsonGustave {
    class XmlWriter {
    private:
        using XmlWriterData = xmlWriter::detail::XmlWriterData;
    public:
        using Attrs = xmlWriter::Attrs;
        using CStringView = jsonGustave::CStringView;
        using Element = xmlWriter::Element;

        using Attr = Attrs::Attr;

        [[nodiscard]]
        explicit XmlWriter(std::FILE* output, bool compactMode = true)
            : data_{ std::make_unique<XmlWriterData>(output, compactMode) }
        {}

        [[nodiscard]]
        Element newRootElement(CStringView name) {
            if (data_->hasOpenedElements()) {
                auto msg = std::format("Can't create a XML root element: the document still has opened elements (depth = {})", data_->depth());
                throw std::logic_error{ std::move(msg) };
            }
            return Element{ *data_, name };
        }

        [[nodiscard]]
        Element newChildElement(CStringView name) {
            return Element{ *data_, name };
        }
    private:
        std::unique_ptr<XmlWriterData> data_;
    };
}
