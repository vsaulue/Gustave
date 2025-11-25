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

#include <string>
#include <string_view>
#include <vector>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgPhaseCanvas.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class LegendBlockType {
    public:
        using Float = G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgDims = detail::SvgDims<Float>;
        using SvgPhaseCanvas = detail::SvgPhaseCanvas<G>;

        using BlockType = SvgCanvasContext::World::BlockType;

        [[nodiscard]]
        explicit LegendBlockType(SvgCanvasContext const& ctx, Color const& blockBorderColor, Float blockBorderWidth, Float xMin, Float yMin)
            : ctx_{ ctx }
            , blockBorderColor_{ blockBorderColor }
            , blockBorderWidth_{ blockBorderWidth }
            , xMin_{ xMin }
            , yMin_{ yMin }
            , dims_{ 0.f, 0.f }
        {
            auto const& srcTypes = ctx.world().blockTypes();
            blockTypes_.reserve(srcTypes.size());
            Float const labelSize = ctx.config().legendTextSize();
            Float labelWidth = 0.f;
            for (BlockType const& blockType : srcTypes) {
                blockTypes_.push_back(&blockType);
                labelWidth = std::max(labelWidth, ctx.textWidth(blockType.name(), labelSize));
            }
            Float const space = ctx.config().legendSpace();
            Float const titleSize = ctx.config().legendTitleSize();
            Float const titleWidth = ctx.textWidth(title(), titleSize);
            Float const lineHeight = std::max(ctx.svgBlockHeight(), labelSize);
            dims_.setWidth(std::max(ctx.svgBlockWidth() + space + labelWidth, titleWidth));
            dims_.setHeight(titleSize + Float(blockTypes_.size()) * (lineHeight + space));
        }

        [[nodiscard]]
        SvgDims const& dims() const {
            return dims_;
        }

        void render(SvgPhaseCanvas& canvas) const {
            Float const space = ctx_.config().legendSpace();
            Float const titleSize = ctx_.config().legendTitleSize();
            std::string const textColor = ctx_.config().legendTextColor().svgCode();
            renderTitle(canvas, titleSize, textColor);
            Float const labelSize = ctx_.config().legendTextSize();
            Float const lineHeight = std::max(ctx_.svgBlockHeight(), labelSize);
            Float const yStep = lineHeight + space;
            Float const yBlocks = yMin_ + titleSize + space + 0.5f * (lineHeight - ctx_.svgBlockHeight());
            Float const xLabels = xMin_ + ctx_.svgBlockWidth() + space;
            Float const yLabels = yMin_ + titleSize + space + 0.5f * (lineHeight + labelSize);
            renderBlocks(canvas, yBlocks, yStep);
            renderLabels(canvas, xLabels, yLabels, yStep, labelSize, textColor);
        }

        [[nodiscard]]
        Float yMax() const {
            return yMin_ + dims_.height();
        }
    private:
        void renderBlocks(SvgPhaseCanvas& canvas, Float yStart, Float yStep) const {
            std::string const borderColorCode = blockBorderColor_.svgCode();
            canvas.startGroup({ {"stroke",borderColorCode},{"stroke-width",blockBorderWidth_} });
            Float y = yStart;
            for (auto const& blockType : blockTypes_) {
                canvas.drawLegendBlock(xMin_, y, { {"fill", blockType->color().svgCode()} });
                y += yStep;
            }
            canvas.endGroup();
        }

        void renderLabels(SvgPhaseCanvas& canvas, Float x, Float yStart, Float yStep, Float textSize, std::string const& color) const {
            canvas.startGroup({ {"fill",color},{"font-size",textSize} });
            Float y = yStart;
            for (auto const& blockType : blockTypes_) {
                canvas.drawLegendText(x, y, blockType->name(), {});
                y += yStep;
            }
            canvas.endGroup();
        }

        void renderTitle(SvgPhaseCanvas& canvas, Float titleSize, std::string const& color) const {
            canvas.drawLegendText(xMin_, yMin_ + titleSize, title(), { {"fill",color},{"font-size",titleSize} });
        }

        [[nodiscard]]
        static std::string_view title() {
            return "Block color (type/material):";
        }

        SvgCanvasContext const& ctx_;
        std::vector<BlockType const*> blockTypes_;
        Color blockBorderColor_;
        Float blockBorderWidth_;
        Float xMin_;
        Float yMin_;
        SvgDims dims_;
    };
}
