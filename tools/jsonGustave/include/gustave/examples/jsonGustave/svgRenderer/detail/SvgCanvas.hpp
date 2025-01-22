/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <span>

#include <svgwrite/writer.hpp>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgLinearGradient.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgRect.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgWorldBox.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/Config.hpp>
#include <gustave/examples/jsonGustave/JsonWorld.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class SvgCanvas {
    public:
        using Float = typename G::RealRep;
        using SyncWorld = typename G::Worlds::SyncWorld;

        using Config = svgRenderer::Config<Float>;
        using JsonWorld = jsonGustave::JsonWorld<G>;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgDims = detail::SvgDims<Float>;
        using SvgLinearGradient = detail::SvgLinearGradient<Float>;
        using SvgRect = detail::SvgRect<Float>;
    private:
        using BlockIndex = typename SyncWorld::BlockIndex;
        using Direction = typename SyncWorld::ContactIndex::Direction;
        using GridCoord = typename SyncWorld::BlockIndex::Coord;

        using SvgWorldBox = detail::SvgWorldBox<G>;
    public:
        class Attrs {
        public:
            using AttrsSpan = std::span<svgw::attr const>;

            [[nodiscard]]
            Attrs(std::initializer_list<svgw::attr> attrs)
                : attrs_{ attrs }
            {}

            [[nodiscard]]
            Attrs(std::convertible_to<AttrsSpan> auto&& attrs)
                : attrs_{ std::forward<decltype(attrs)>(attrs) }
            {}

            [[nodiscard]]
            operator AttrsSpan() const {
                return attrs_;
            }
        private:
            AttrsSpan attrs_;
        };

        [[nodiscard]]
        explicit SvgCanvas(SvgCanvasContext const& ctx, SvgDims const& legendDims, std::ostream& output)
            : ctx_{ ctx }
            , output_{ output }
            , worldBox_{ ctx }
            , writer_{ output }
            , groupCount_{ 0 }
            , finalized_{ false }
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
            writer_.start_svg(svgWidth, svgHeight);
        }

        SvgCanvas(SvgCanvas const&) = delete;
        SvgCanvas(SvgCanvas&&) = delete;

        [[nodiscard]]
        std::string defLinearGradient(SvgLinearGradient const& grad) {
            writer_.start_defs();
            std::string defId = std::format("linGrad_{}", nextDefId_);
            ++nextDefId_;
            std::string rawSvg = std::format(R"(<linearGradient id="{}" x1="{}" x2="{}" y1="{}" y2="{}">)",
                defId, grad.x1(), grad.x2(), grad.y1(), grad.y2()
            );
            writer_.write(rawSvg);
            for (auto const& stop : grad.stops()) {
                rawSvg = std::format(R"(<stop offset="{}" stop-color="{}" />)", stop.offset, stop.color.svgCode());
                writer_.write(rawSvg);
            }
            writer_.write("</linearGradient>");
            writer_.end_defs();
            return defId;
        }

        void drawLegendBlock(Float xMin, Float yMin, Attrs attrs) {
            throwIfFinalized();
            auto const coords = SvgRect{ xMin, yMin, ctx_.svgBlockWidth(), ctx_.svgBlockHeight() };
            drawBlock(coords, attrs);
        }

        void drawLegendContactArrow(Float xMin, Float yMin, Float lengthRatio, Attrs attrs) {
            throwIfFinalized();
            auto const blockCoords = SvgRect{ xMin, yMin, ctx_.svgBlockWidth(), ctx_.svgBlockHeight() };
            drawContactArrow(blockCoords, Direction::minusX(), lengthRatio, attrs);
        }

        void drawLegendLine(Float x1, Float y1, Float x2, Float y2, Attrs attrs) {
            throwIfFinalized();
            writer_.line(x1, y1, x2, y2, attrs);
        }

        void drawLegendRect(SvgRect const& rect, Attrs attrs) {
            throwIfFinalized();
            writer_.rect(rect.xMin(), rect.yMin(), rect.width(), rect.height(), attrs);
        }

        void drawLegendText(Float xMin, Float yMax, std::string_view text, Attrs attrs) {
            throwIfFinalized();
            writer_.text(xMin, yMax, text, attrs);
        }

        void drawWorldBlock(SyncWorld::BlockReference const& block, Attrs attrs) {
            throwIfFinalized();
            auto const coords = worldBox_.blockCoordinates(block.index());
            drawBlock(coords, attrs);
        }

        void drawWorldContactArrow(SyncWorld::ContactReference const& contact, Float lengthRatio, Attrs attrs) {
            throwIfFinalized();
            auto const blockCoords = worldBox_.blockCoordinates(contact.localBlock().index());
            auto const direction = contact.index().direction();
            drawContactArrow(blockCoords, direction, lengthRatio, attrs);
        }

        void drawWorldFrame(Attrs attrs) {
            throwIfFinalized();
            SvgRect const box = worldBox_.boxCoordinates();
            writer_.rect(box.xMin(), box.yMin(), box.width(), box.height(), attrs);
        }

        void endGroup() {
            throwIfFinalized();
            if (groupCount_ == 0) {
                throw std::logic_error("Invalid endGroup(): no group to close.");
            }
            --groupCount_;
            writer_.end_g();
        }

        void finalize() {
            throwIfFinalized();
            if (groupCount_ > 0) {
                throw std::logic_error("Invalid finalize(): all groups aren't closed.");
            }
            finalized_ = true;
            writer_.end_svg();
            output_ << '\n';
        }

        void hatchLegendBlock(Float xMin, Float yMin, Attrs attrs) {
            throwIfFinalized();
            auto const coords = SvgRect{ xMin, yMin, ctx_.svgBlockWidth(), ctx_.svgBlockHeight() };
            hatchBlock(coords, attrs);
        }

        void hatchWorldBlock(SyncWorld::BlockReference const& block, Attrs attrs) {
            throwIfFinalized();
            auto const coords = worldBox_.blockCoordinates(block.index());
            hatchBlock(coords, attrs);
        }

        void startGroup(Attrs attrs) {
            throwIfFinalized();
            writer_.start_g(attrs);
            ++groupCount_;
        }

        [[nodiscard]]
        SvgWorldBox const& worldBox() const {
            return worldBox_;
        }
    private:
        void drawBlock(SvgRect const& coords, Attrs attrs) {
            writer_.rect(coords.xMin(), coords.yMin(), coords.width(), coords.height(), attrs);
        }

        void drawContactArrow(SvgRect const& blockCoords, Direction direction, Float lengthRatio, Attrs attrs) {
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
            writer_.path(path.str(), attrs);
        }

        void hatchBlock(SvgRect const& coords, Attrs attrs) {
            writer_.start_g(attrs);
            writer_.line(coords.xMean(), coords.yMin(), coords.xMax(), coords.yMean());
            writer_.line(coords.xMin(), coords.yMin(), coords.xMax(), coords.yMax());
            writer_.line(coords.xMin(), coords.yMean(), coords.xMean(), coords.yMax());
            writer_.end_g();
        }

        void throwIfFinalized() const {
            if (finalized_) {
                throw std::logic_error("SvgCanvas finalized twice.");
            }
        }

        SvgCanvasContext const& ctx_;
        std::ostream& output_;
        SvgWorldBox worldBox_;
        svgw::writer writer_;
        std::size_t nextDefId_ = 0;
        unsigned groupCount_;
        bool finalized_;
    };
}
