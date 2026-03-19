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

#include <concepts>
#include <cstdio>
#include <span>
#include <variant>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgLinearGradient.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgRect.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgWorldBox.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/Config.hpp>
#include <gustave/examples/jsonGustave/JsonWorld.hpp>
#include <gustave/examples/jsonGustave/XmlWriter.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class SvgCanvas {
    public:
        using Float = G::RealRep;
        using SyncWorld = G::Worlds::SyncWorld;

        using Config = svgRenderer::Config<Float>;
        using JsonWorld = jsonGustave::JsonWorld<G>;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgDims = detail::SvgDims<Float>;
        using SvgLinearGradient = detail::SvgLinearGradient<Float>;
        using SvgRect = detail::SvgRect<Float>;
    private:
        using BlockIndex = SyncWorld::BlockIndex;
        using Direction = SyncWorld::ContactIndex::Direction;
        using GridCoord = SyncWorld::BlockIndex::Coord;

        using SvgWorldBox = detail::SvgWorldBox<G>;
    public:
        using XmlAttr = XmlWriter::Attr;
        using XmlAttrs = XmlWriter::Attrs;
        using XmlElement = XmlWriter::Element;

        [[nodiscard]]
        explicit SvgCanvas(SvgCanvasContext const& ctx, SvgDims const& legendDims, std::FILE* output)
            : ctx_{ ctx }
            , worldBox_{ ctx }
            , xmlWriter_{ output, true }
            , xmlRoot_{ xmlWriter_.newRootElement("svg") }
        {
            for (auto const& block : ctx.world().syncWorld().blocks()) {
                if (block.index().z != 0) {
                    std::stringstream msg;
                    msg << "SvgRenderer doesn't support 3d scenes: all blocks must hase 'index.z == 0' (passed: " << block.index() << ").";
                    throw std::invalid_argument(msg.str());
                }
            }
            SvgRect const worldRect = worldBox_.boxCoordinates();
            Float const svgWidth = std::max(worldRect.width(), legendDims.width());
            Float const svgHeight = worldRect.height() + legendDims.height();
            xmlRoot_.attr("xmlns", "http://www.w3.org/2000/svg");
            xmlRoot_.attr("xmlns:xlink", "http://www.w3.org/1999/xlink");
            xmlRoot_.attr("version", "1.1");
            xmlRoot_.attr("width", svgWidth);
            xmlRoot_.attr("height", svgHeight);
        }

        SvgCanvas(SvgCanvas const&) = delete;
        SvgCanvas(SvgCanvas&&) = delete;

        [[nodiscard]]
        std::string defLinearGradient(SvgLinearGradient const& grad) {
            auto defId = std::format("linGrad_{}", nextDefId_);
            ++nextDefId_;
            auto xmlDefs = xmlElement("defs");
            auto xmlLinGrad = xmlDefs.newElement("linearGradient");
            xmlLinGrad.attr("id", defId);
            xmlLinGrad.attr("x1", grad.x1());
            xmlLinGrad.attr("x2", grad.x2());
            xmlLinGrad.attr("y1", grad.y1());
            xmlLinGrad.attr("y2", grad.y2());
            for (auto const& stop : grad.stops()) {
                auto xmlStop = xmlLinGrad.newElement("stop");
                xmlStop.attr("offset", stop.offset);
                xmlStop.attr("stop-color", stop.color.svgCode());
                xmlStop.close();
            }
            xmlLinGrad.close();
            xmlDefs.close();
            return defId;
        }

        void drawLegendBlock(Float xMin, Float yMin, XmlAttrs attrs) {
            throwIfClosed();
            auto const coords = SvgRect{ xMin, yMin, ctx_.svgBlockWidth(), ctx_.svgBlockHeight() };
            svgRect(coords, attrs);
        }

        void drawLegendContactArrow(Float xMin, Float yMin, Float lengthRatio, XmlAttrs attrs) {
            throwIfClosed();
            auto const blockCoords = SvgRect{ xMin, yMin, ctx_.svgBlockWidth(), ctx_.svgBlockHeight() };
            drawContactArrow(blockCoords, Direction::minusX(), lengthRatio, attrs);
        }

        void drawLegendLine(Float x1, Float y1, Float x2, Float y2, XmlAttrs attrs) {
            throwIfClosed();
            svgLine(x1, y1, x2, y2, attrs);
        }

        void drawLegendRect(SvgRect const& rect, XmlAttrs attrs) {
            throwIfClosed();
            svgRect(rect, attrs);
        }

        void drawLegendText(Float xMin, Float yMax, CStringView text, XmlAttrs attrs) {
            throwIfClosed();
            svgText(xMin, yMax, text, attrs);
        }

        void drawWorldBlock(SyncWorld::BlockReference const& block, XmlAttrs attrs) {
            throwIfClosed();
            auto const coords = worldBox_.blockCoordinates(block.index());
            svgRect(coords, attrs);
        }

        void drawWorldContactArrow(SyncWorld::ContactReference const& contact, Float lengthRatio, XmlAttrs attrs) {
            throwIfClosed();
            auto const blockCoords = worldBox_.blockCoordinates(contact.localBlock().index());
            auto const direction = contact.index().direction();
            drawContactArrow(blockCoords, direction, lengthRatio, attrs);
        }

        void drawWorldFrame(XmlAttrs attrs) {
            throwIfClosed();
            svgRect(worldBox_.boxCoordinates(), attrs);
        }

        void finalize() {
            throwIfClosed();
            xmlRoot_.close();
        }

        void hatchLegendBlock(Float xMin, Float yMin, XmlAttrs attrs) {
            throwIfClosed();
            auto const coords = SvgRect{ xMin, yMin, ctx_.svgBlockWidth(), ctx_.svgBlockHeight() };
            hatchBlock(coords, attrs);
        }

        void hatchWorldBlock(SyncWorld::BlockReference const& block, XmlAttrs attrs) {
            throwIfClosed();
            auto const coords = worldBox_.blockCoordinates(block.index());
            hatchBlock(coords, attrs);
        }

        [[nodiscard]]
        XmlElement svgGroup(XmlAttrs attrs) {
            throwIfClosed();
            auto result = xmlElement("g");
            result.attrs(attrs);
            return result;
        }

        [[nodiscard]]
        SvgWorldBox const& worldBox() const {
            return worldBox_;
        }
    private:
        void drawContactArrow(SvgRect const& blockCoords, Direction direction, Float lengthRatio, XmlAttrs attrs) {
            Float const triangleFactor = ctx_.config().arrowTriangleFactor();
            Float const minDim = std::min(blockCoords.height(), blockCoords.width());
            Float const triangleSize = minDim * triangleFactor;
            Float const lineWidth = triangleSize * ctx_.config().arrowLineFactor();
            Float const lineLength = minDim * (0.5f - triangleFactor) * lengthRatio;
            std::stringstream path;
            switch (direction.id()) {
            case Direction::Id::minusX: {
                Float const xTriangleBase = blockCoords.xMin() + lineLength;
                Float const yMean = blockCoords.yMean();
                path << 'M' << xTriangleBase << ' ' << yMean - triangleSize;
                path << " L" << xTriangleBase + triangleSize << ' ' << yMean;
                path << " L" << xTriangleBase << ' ' << yMean + triangleSize;
                path << " L" << xTriangleBase << ' ' << yMean + lineWidth;
                path << " L" << blockCoords.xMin() << ' ' << yMean + lineWidth;
                path << " L" << blockCoords.xMin() << ' ' << yMean - lineWidth;
                path << " L" << xTriangleBase << ' ' << yMean - lineWidth;
                break;
            }
            case Direction::Id::plusX: {
                Float const xMax = blockCoords.xMax();
                Float const xTriangleBase = xMax - lineLength;
                Float const yMean = blockCoords.yMean();
                path << 'M' << xTriangleBase << ' ' << yMean - triangleSize;
                path << " L" << xTriangleBase - triangleSize << ' ' << yMean;
                path << " L" << xTriangleBase << ' ' << yMean + triangleSize;
                path << " L" << xTriangleBase << ' ' << yMean + lineWidth;
                path << " L" << xMax << ' ' << yMean + lineWidth;
                path << " L" << xMax << ' ' << yMean - lineWidth;
                path << " L" << xTriangleBase << ' ' << yMean - lineWidth;
                break;
            }
            case Direction::Id::minusY: {
                Float const xMean = blockCoords.xMean();
                Float const yMax = blockCoords.yMax();
                Float const yTriangleBase = yMax - lineLength;
                path << 'M' << xMean - triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean << ' ' << yTriangleBase - triangleSize;
                path << " L" << xMean + triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << yMax;
                path << " L" << xMean - lineWidth << ' ' << yMax;
                path << " L" << xMean - lineWidth << ' ' << yTriangleBase;
                break;
            }
            case Direction::Id::plusY: {
                Float const xMean = blockCoords.xMean();
                Float const yTriangleBase = blockCoords.yMin() + lineLength;
                path << 'M' << xMean - triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean << ' ' << yTriangleBase + triangleSize;
                path << " L" << xMean + triangleSize << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << yTriangleBase;
                path << " L" << xMean + lineWidth << ' ' << blockCoords.yMin();
                path << " L" << xMean - lineWidth << ' ' << blockCoords.yMin();
                path << " L" << xMean - lineWidth << ' ' << yTriangleBase;
                break;
            }
            case Direction::Id::minusZ:
            case Direction::Id::plusZ: {
                std::stringstream msg;
                msg << "Unsupported direction for SvgRender:" << direction;
                throw std::invalid_argument(msg.str());
            }
            }
            path << " Z";
            auto xmlPath = xmlElement("path");
            xmlPath.attr("d", path.str());
            xmlPath.attrs(attrs);
            xmlPath.close();
        }

        void hatchBlock(SvgRect const& coords, XmlAttrs attrs) {
            auto xmlGroup = svgGroup(attrs);
            svgLine(coords.xMean(), coords.yMin(), coords.xMax(), coords.yMean());
            svgLine(coords.xMin(), coords.yMin(), coords.xMax(), coords.yMax());
            svgLine(coords.xMin(), coords.yMean(), coords.xMean(), coords.yMax());
            xmlGroup.close();
        }

        void svgLine(Float x1, Float y1, Float x2, Float y2, XmlAttrs attrs = {}) {
            auto xmlLine = xmlElement("line");
            xmlLine.attr("x1", x1);
            xmlLine.attr("y1", y1);
            xmlLine.attr("x2", x2);
            xmlLine.attr("y2", y2);
            xmlLine.attrs(attrs);
            xmlLine.close();
        }

        void svgRect(SvgRect const& rect, XmlAttrs attrs = {}) {
            auto xmlRect = xmlElement("rect");
            xmlRect.attr("x", rect.xMin());
            xmlRect.attr("y", rect.yMin());
            xmlRect.attr("width", rect.width());
            xmlRect.attr("height", rect.height());
            xmlRect.attrs(attrs);
            xmlRect.close();
        }

        void svgText(Float x, Float y, CStringView text, XmlAttrs attrs) {
            auto xmlText = xmlElement("text");
            xmlText.attr("x", x);
            xmlText.attr("y", y);
            xmlText.attrs(attrs);
            xmlText.text(text);
            xmlText.close();
        }

        void throwIfClosed() const {
            if (xmlRoot_.isClosed()) {
                throw std::logic_error("SvgCanvas closed twice.");
            }
        }

        [[nodiscard]]
        XmlElement xmlElement(CStringView name) {
            return xmlWriter_.newChildElement(name);
        }

        SvgCanvasContext const& ctx_;
        SvgWorldBox worldBox_;
        XmlWriter xmlWriter_;
        std::size_t nextDefId_ = 0;
        XmlElement xmlRoot_;
    };
}
