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

#include <string>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgPhaseCanvas.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class LegendContactLength {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgDims = detail::SvgDims<Float>;
        using SvgPhaseCanvas = detail::SvgPhaseCanvas<G>;

        [[nodiscard]]
        explicit LegendContactLength(
            SvgCanvasContext const& ctx,
            Float contactStrokeWidth,
            Color const& contactStrokeColor,
            Color const& contactFillColor,
            std::string title,
            std::string minLabel,
            std::string maxLabel,
            Float xMin,
            Float yMin
        )
            : ctx_{ ctx }
            , xMin_{ xMin }
            , yMin_{ yMin }
            , contactStrokeWidth_{ contactStrokeWidth }
            , contactStrokeColor_{ contactStrokeColor }
            , contactFillColor_{ contactFillColor }
            , title_{ std::move(title) }
            , minLabel_{ std::move(minLabel) }
            , maxLabel_{ std::move(maxLabel) }
            , dims_{ 0.f, 0.f }
        {
            dims_ = computeDims();
        }

        [[nodiscard]]
        SvgDims const& dims() const {
            return dims_;
        }

        void render(SvgPhaseCanvas& canvas) const {
            Float const space = ctx_.config().legendSpace();
            Float const titleSize = ctx_.config().legendTitleSize();
            Float const labelSize = ctx_.config().legendTextSize();
            Float const blockHeight = ctx_.svgBlockHeight();
            Float const lineHeight = std::max(labelSize, blockHeight);
            Float const yStep = lineHeight + space;
            std::string const textColor = ctx_.config().legendTextColor().svgCode();
            Float const yBlock = yMin_ + titleSize + space + 0.5f * (lineHeight - blockHeight);
            Float const xLabels = xMin_ + ctx_.svgBlockWidth() + space;
            Float const yLabels = yMin_ + titleSize + space + 0.5f * (lineHeight + labelSize);
            renderTitle(canvas, titleSize, textColor);
            renderBlocks(canvas, yBlock, yStep, textColor);
            renderContacts(canvas, yBlock, yStep);
            renderLabels(canvas, xLabels, yLabels, yStep, labelSize, textColor);
        }
    private:
        [[nodiscard]]
        SvgDims computeDims() const {
            Float const space = ctx_.config().legendSpace();
            Float const labelSize = ctx_.config().legendTextSize();
            Float const labelWidth = std::max(ctx_.textWidth(minLabel_, labelSize), ctx_.textWidth(maxLabel_, labelSize));
            Float const titleSize = ctx_.config().legendTitleSize();
            Float const titleWidth = ctx_.textWidth(title_, titleSize);
            Float const width = std::max(titleWidth, ctx_.svgBlockWidth() + space + labelWidth);
            Float const itemHeight = std::max(labelSize, ctx_.svgBlockHeight());
            Float const height = titleSize + 2.f * (itemHeight + space);
            return { width, height };
        }

        void renderBlocks(SvgPhaseCanvas& canvas, Float yStart, Float yStep, std::string const& blockStrokeColor) const {
            canvas.startGroup({ {"stroke-width",1}, { "stroke", blockStrokeColor }, {"fill-opacity",0.f} });
            canvas.drawLegendBlock(xMin_, yStart, {});
            canvas.drawLegendBlock(xMin_, yStart + yStep, {});
            canvas.endGroup();
        }

        void renderContacts(SvgPhaseCanvas& canvas, Float yStart, Float yStep) const {
            std::string const fillColorCode = contactFillColor_.svgCode();
            std::string const strokeColorCode = contactStrokeColor_.svgCode();
            canvas.startGroup({ {"stroke-width", contactStrokeWidth_}, {"stroke",strokeColorCode}, {"fill",fillColorCode} });
            canvas.drawLegendContactArrow(xMin_, yStart, 0.f, {});
            canvas.drawLegendContactArrow(xMin_, yStart + yStep, 1.f, {});
            canvas.endGroup();
        }

        void renderLabels(SvgPhaseCanvas& canvas, Float xMin, Float yStart, Float yStep, Float labelSize, std::string const& textColor) const {
            canvas.startGroup({ {"font-size",labelSize},{"fill",textColor} });
            canvas.drawLegendText(xMin, yStart, minLabel_, {});
            canvas.drawLegendText(xMin, yStart + yStep, maxLabel_, {});
            canvas.endGroup();
        }

        void renderTitle(SvgPhaseCanvas& canvas, Float titleSize, std::string const& textColor) const {
            canvas.drawLegendText(xMin_, yMin_ + titleSize, title_, { {"font-size",titleSize},{"fill", textColor} });
        }

        SvgCanvasContext const& ctx_;
        Float xMin_;
        Float yMin_;
        Float contactStrokeWidth_;
        Color contactStrokeColor_;
        Color contactFillColor_;
        std::string title_;
        std::string minLabel_;
        std::string maxLabel_;
        SvgDims dims_;
    };
}
