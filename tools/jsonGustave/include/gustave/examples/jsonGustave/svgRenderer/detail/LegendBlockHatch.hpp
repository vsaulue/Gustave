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
#include <string_view>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgPhaseCanvas.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class LegendBlockHatch {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgDims = detail::SvgDims<Float>;
        using SvgPhaseCanvas = detail::SvgPhaseCanvas<G>;

        [[nodiscard]]
        explicit LegendBlockHatch(SvgCanvasContext const& ctx, Color const& hatchColor, Float hatchWidth, Float xMin, Float yMin)
            : ctx_{ ctx }
            , xMin_{ xMin }
            , yMin_{ yMin }
            , hatchWidth_{ hatchWidth }
            , hatchColor_{ hatchColor }
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
            Float const lineHeight = std::max(labelSize, ctx_.svgBlockHeight());
            std::string const textColor = ctx_.config().legendTextColor().svgCode();
            canvas.drawLegendText(xMin_, yMin_ + titleSize, title(), { {"font-size",titleSize}, {"fill",textColor} });
            Float const yBlock = yMin_ + titleSize + space + 0.5f * (lineHeight - ctx_.svgBlockHeight());
            canvas.drawLegendBlock(xMin_, yBlock, { {"fill-opacity",0.f},{"stroke",textColor},{"stroke-width",1.f} });
            std::string const hatchColorCode = hatchColor_.svgCode();
            canvas.hatchLegendBlock(xMin_, yBlock, { {"stroke",hatchColorCode},{"stroke-width",hatchWidth_} });
            Float const xLabel = xMin_ + ctx_.svgBlockWidth() + space;
            Float const yLabel = yMin_ + titleSize + space + 0.5f * (lineHeight + labelSize);
            canvas.drawLegendText(xLabel, yLabel, label(), { {"font-size",labelSize},{"fill",textColor}});
        }
    private:
        [[nodiscard]]
        SvgDims computeDims() const {
            Float const space = ctx_.config().legendSpace();
            Float const titleSize = ctx_.config().legendTitleSize();
            Float const labelSize = ctx_.config().legendTextSize();
            Float const titleWidth = ctx_.textWidth(title(), titleSize);
            Float const labelWidth = ctx_.textWidth(label(), labelSize);
            Float const lineWidth = ctx_.svgBlockWidth() + space + labelWidth;
            Float const lineHeight = std::max(labelSize, ctx_.svgBlockHeight());
            Float const width = std::max(titleWidth, lineWidth);
            return { width, titleSize + space + lineHeight};
        }

        [[nodiscard]]
        static std::string_view label() {
            return "Foundation";
        }

        [[nodiscard]]
        static std::string_view title() {
            return "Block hatching:";
        }

        SvgCanvasContext const& ctx_;
        Float xMin_;
        Float yMin_;
        Float hatchWidth_;
        Color hatchColor_;
        SvgDims dims_;
    };
}
