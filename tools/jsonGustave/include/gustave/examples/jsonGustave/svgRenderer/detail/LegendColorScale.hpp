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

#include <cassert>
#include <format>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgLinearGradient.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgPhaseCanvas.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgRect.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/ColorScale.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class LegendColorScale {
    public:
        using Float = G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using ColorScale = svgRenderer::ColorScale<Float>;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgDims = detail::SvgDims<Float>;
        using SvgPhaseCanvas = detail::SvgPhaseCanvas<G>;
        using SvgRect = detail::SvgRect<Float>;
    private:
        using SvgLinearGradient = detail::SvgLinearGradient<Float>;

        using GradientStop = SvgLinearGradient::Stop;

        struct GradientRect {
        public:
            Float height;
            std::string endLabel;
            Color startColor;
            Color endColor;
        };
    public:
        [[nodiscard]]
        explicit LegendColorScale(ColorScale const& scale, SvgCanvasContext const& ctx, std::string title, Float xMin, Float yMin)
            : ctx_{ ctx }
            , xMin_{ xMin }
            , yMin_{ yMin }
            , firstColor_{ scale.colorPoints().front().colorBefore }
            , lastColor_{ scale.colorPoints().back().colorAfter }
            , title_{ std::move(title) }
            , firstLabel_{ label(scale.colorPoints().front().index) }
            , dims_{ 0.f, 0.f }
        {
            rects_.reserve(scale.colorPoints().size() - 1);
            auto const itEnd = scale.colorPoints().end() - 1;
            auto itPoint = scale.colorPoints().begin();
            while (itPoint != itEnd) {
                auto const nextIt = itPoint + 1;
                Float const rHeight = Float(nextIt->index - itPoint->index) * ctx.config().legendColorScaleRes();
                rects_.push_back({
                    .height = rHeight,
                    .endLabel = label(nextIt->index),
                    .startColor = itPoint->colorAfter,
                    .endColor = nextIt->colorBefore,
                });
                itPoint = nextIt;
            }
            dims_ = computeDims();
        }

        [[nodiscard]]
        SvgDims const& dims() const {
            return dims_;
        }

        void render(SvgPhaseCanvas& canvas) const {
            renderTitle(canvas);
            Float const yOffset = yMin_ + ctx_.config().legendSpace() + ctx_.config().legendTitleSize();
            renderGraduations(canvas, yOffset);
            renderScale(canvas, yOffset);
            renderLabels(canvas, yOffset);
        }

        [[nodiscard]]
        Float yMax() const {
            return yMin_ + dims_.height();
        }
    private:
        [[nodiscard]]
        SvgDims computeDims() const {
            Float const space = ctx_.config().legendSpace();
            Float textWidth = labelWidth(firstLabel_);
            Float height = space + ctx_.config().legendTitleSize() + ctx_.config().legendTextSize();
            for (GradientRect const& rect : rects_) {
                textWidth = std::max(textWidth, labelWidth(rect.endLabel));
                height += rect.height;
            }
            Float const scaleWidth = textWidth + 2 * space + ctx_.config().legendColorScaleWidth();
            Float const width = std::max(scaleWidth, ctx_.textWidth(title_, ctx_.config().legendTitleSize()));
            return { width, height };
        }

        [[nodiscard]]
        static std::string label(Float f) {
            return std::format("{:.2f}%", 100.f * f);
        }

        [[nodiscard]]
        Float labelWidth(std::string_view value) const {
            return ctx_.textWidth(value, ctx_.config().legendTextSize());
        }

        void renderGraduations(SvgPhaseCanvas& canvas, Float yStart) const {
            Float const space = ctx_.config().legendSpace();
            Float const scaleWidth = ctx_.config().legendColorScaleWidth();
            auto const textColor = ctx_.config().legendTextColor().svgCode();
            Float const textSize = ctx_.config().legendTextSize();
            Float const xMax = xMin_ + scaleWidth + space;
            Float yOffset = yStart + 0.5f * textSize;
            canvas.startGroup({ {"stroke", textColor}, {"stroke-width", 1.f} });
            canvas.drawLegendLine(xMin_, yOffset, xMax, yOffset, {});
            for (GradientRect const& gRect : rects_) {
                yOffset += gRect.height;
                canvas.drawLegendLine(xMin_, yOffset, xMax, yOffset, {});
            }
            canvas.endGroup();
        }

        void renderLabels(SvgPhaseCanvas& canvas, Float yStart) const {
            Float const space = ctx_.config().legendSpace();
            Float const textSize = ctx_.config().legendTextSize();
            auto const textColorCode = ctx_.config().legendTextColor().svgCode();
            Float const xOffset = xMin_ + ctx_.config().legendColorScaleWidth() + 2.f * space;
            Float yOffset = yStart + textSize;
            canvas.startGroup({ {"font-size", textSize}, {"fill", textColorCode} });
            canvas.drawLegendText(xOffset, yOffset, firstLabel_, {});
            for (GradientRect const& gRect : rects_) {
                yOffset += gRect.height;
                canvas.drawLegendText(xOffset, yOffset, gRect.endLabel, {});
            }
            canvas.endGroup();
        }

        void renderScale(SvgPhaseCanvas& canvas, Float yStart) const {
            Float const width = ctx_.config().legendColorScaleWidth();
            Float const extremityHeight = 0.5f * ctx_.config().legendTextSize();
            std::string color = firstColor_.svgCode();
            std::array<svgw::attr, 1> attrs = { {
                { "fill", color },
            } };
            auto rect = SvgRect{ xMin_, yStart, width, extremityHeight };
            canvas.drawLegendRect(rect, attrs);
            std::array<GradientStop, 2> stops{ {
                {0.f, Color{0.f, 0.f, 0.f}},
                {1.f, Color{0.f, 0.f, 0.f}},
            } };
            auto gradDef = SvgLinearGradient{ 0.f, 0.f, 0.f, 1.f, stops };
            for (GradientRect const& gRect : rects_) {
                stops[0].color = gRect.startColor;
                stops[1].color = gRect.endColor;
                std::string gradId = canvas.defLinearGradient(gradDef);
                rect.yMin() += rect.height();
                rect.setHeight(gRect.height);
                color = "url(#" + gradId + ")";
                attrs[0].value = color;
                canvas.drawLegendRect(rect, attrs);
            }
            rect.yMin() += rect.height();
            rect.setHeight(extremityHeight);
            color = lastColor_.svgCode();
            attrs[0].value = color;
            canvas.drawLegendRect(rect, attrs);
        }

        void renderTitle(SvgPhaseCanvas& canvas) const {
            auto const titleSize = ctx_.config().legendTitleSize();
            canvas.drawLegendText(xMin_, yMin_ + titleSize, title_, {
                {"font-size", titleSize},
                {"fill", ctx_.config().legendTextColor().svgCode()},
            });
        }

        SvgCanvasContext const& ctx_;
        Float xMin_;
        Float yMin_;
        Color firstColor_;
        Color lastColor_;
        std::string title_;
        std::string firstLabel_;
        std::vector<GradientRect> rects_;
        SvgDims dims_;
    };
}
