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

#include <format>
#include <stdexcept>
#include <utility>

#include <gustave/examples/jsonGustave/xmlWriter/detail/XmlWriterData.hpp>
#include <gustave/examples/jsonGustave/xmlWriter/Attrs.hpp>
#include <gustave/examples/jsonGustave/CStringView.hpp>
#include <gustave/utils/prop/Ptr.hpp>

namespace gustave::examples::jsonGustave::xmlWriter {
    class Element {
    public:
        using Attrs = xmlWriter::Attrs;
        using CStringView = jsonGustave::CStringView;
        using XmlWriterData = detail::XmlWriterData;

        using Depth = XmlWriterData::Depth;

        [[nodiscard]]
        explicit Element(XmlWriterData& writer, CStringView name)
            : writer_{ &writer }
            , depth_{ writer.newElement(name) }
        {}

        Element(Element const&) = delete;
        Element& operator=(Element const&) = delete;

        Element(Element&&) = default;
        Element& operator=(Element&&) = default;

        void attr(CStringView name, auto const& value)
            requires requires (XmlWriterData& d) { d.attr(name, value); }
        {
            throwIfNotActive();
            writer_->attr(name, value);
        }

        void attrs(Attrs attributes) {
            throwIfNotActive();
            for (auto const& attr : attributes) {
                attr.value.visit([&](auto const& attrValue) { writer_->attr(attr.name, attrValue); });
            }
        }

        void close() {
            throwIfNotActive();
            writer_->closeElement();
            writer_ = nullptr;
        }

        [[nodiscard]]
        bool isClosed() const {
            return writer_ == nullptr;
        }

        [[nodiscard]]
        Element newElement(CStringView name) {
            throwIfNotActive();
            return Element{ *writer_, name };
        }

        void text(CStringView content) {
            throwIfNotActive();
            writer_->text(content);
        }
    private:
        void throwIfInvalid() const {
            if (isClosed()) {
                throw std::logic_error{ "XML element was already closed." };
            }
        }

        void throwIfNotActive() const {
            throwIfInvalid();
            auto const wDepth = writer_->depth();
            if (depth_ != wDepth) {
                auto msg = std::format("XML element still has opened children (element's depth: {} / writer's depth: {}).", depth_, wDepth);
                throw std::logic_error{ std::move(msg) };
            }
        }

        utils::prop::Ptr<XmlWriterData> writer_;
        Depth depth_;
    };
}
