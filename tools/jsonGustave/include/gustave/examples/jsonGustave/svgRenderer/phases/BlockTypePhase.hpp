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

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/LegendBlockHatch.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/LegendBlockType.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class BlockTypePhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using Config = typename Phase<G>::Config;
        using JsonWorld = typename Phase<G>::JsonWorld;
        using SvgCanvasContext = typename Phase<G>::SvgCanvasContext;
        using SvgDims = typename Phase<G>::SvgDims;
        using SvgPhaseCanvas = typename Phase<G>::SvgPhaseCanvas;
        using PhaseContext = typename Phase<G>::PhaseContext;
    private:
        using LegendBlockHatch = detail::LegendBlockHatch<G>;
        using LegendBlockType = detail::LegendBlockType<G>;
    public:

        class BlockTypePhaseContext : public PhaseContext {
        public:
            [[nodiscard]]
            explicit BlockTypePhaseContext(SvgCanvasContext const& ctx, BlockTypePhase const& phase)
                : PhaseContext{ ctx }
                , phase_{ phase }
                , legendType_{ ctx, phase.blockBorderColor_, phase.blockBorderWidth_, 0.f, 0.f }
                , legendHatch_{ ctx, phase.foundationHatchColor_, phase.foundationHatchWidth_, 0.f, legendType_.yMax() + ctx.config().legendSpace() }
            {
                this->setLegendDims(generateLegendDims());
            }
        protected:
            void renderLegend(SvgPhaseCanvas& canvas) const override {
                legendType_.render(canvas);
                legendHatch_.render(canvas);
            }

            void renderWorld(SvgPhaseCanvas& canvas) const override {
                canvas.startGroup({ {"stroke", phase_.blockBorderColor_.svgCode()}, {"stroke-width", phase_.blockBorderWidth_} });
                auto const hatchColorCode = phase_.foundationHatchColor_.svgCode();
                for (auto const& block : this->syncWorld().blocks()) {
                    auto const svgColor = this->jsonWorld().blockTypeOf().at(block.index())->color().svgCode();
                    canvas.drawWorldBlock(block, { {"fill", svgColor } });
                    if (block.isFoundation()) {
                        canvas.hatchWorldBlock(block, { {"stroke", hatchColorCode },{"stroke-width", phase_.foundationHatchWidth_} });
                    }
                }
                canvas.endGroup();
            }
        private:
            [[nodiscard]]
            SvgDims generateLegendDims() const {
                SvgDims const& typeDims = legendType_.dims();
                SvgDims const& hatchDims = legendHatch_.dims();
                Float const width = std::max(typeDims.width(), hatchDims.width());
                Float const height = typeDims.height() + hatchDims.height() + this->config().legendSpace();
                return { width, height };
            }

            BlockTypePhase const& phase_;
            LegendBlockType legendType_;
            LegendBlockHatch legendHatch_;
        };

        [[nodiscard]]
        BlockTypePhase()
            : blockBorderColor_{ 0.f, 0.f, 0.f }
            , foundationHatchColor_{ 0.f, 0.f, 0.f }
            , blockBorderWidth_{ 1.f }
            , foundationHatchWidth_{ 2.f }
        {}

        [[nodiscard]]
        explicit BlockTypePhase(Color const& blockBorderColor, Float blockBorderWidth, Color const& foundationHatchColor, Float foundationHatchWidth)
            : blockBorderColor_{ blockBorderColor }
            , foundationHatchColor_{ foundationHatchColor }
            , blockBorderWidth_{ blockBorderWidth }
            , foundationHatchWidth_{ foundationHatchWidth }
        {
            if (blockBorderWidth_ < 0.f) {
                throw Phase<G>::invalidWidthError("blockBorderWidth", blockBorderWidth_);
            }
            if (foundationHatchWidth_ < 0.f) {
                throw Phase<G>::invalidWidthError("foundationHatchWidth", foundationHatchWidth_);
            }
        }

        [[nodiscard]]
        std::unique_ptr<PhaseContext> makeContext(SvgCanvasContext const& canvasCtx) const override {
            return std::make_unique<BlockTypePhaseContext>(canvasCtx, *this);
        }
    private:
        Color blockBorderColor_;
        Color foundationHatchColor_;
        Float blockBorderWidth_;
        Float foundationHatchWidth_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::phases::BlockTypePhase<G>> {
    using BlockTypePhase = gustave::examples::jsonGustave::svgRenderer::phases::BlockTypePhase<G>;

    using Color = typename BlockTypePhase::Color;
    using Float = typename BlockTypePhase::Float;

    [[nodiscard]]
    static BlockTypePhase from_json(nlohmann::json const& json) {
        Color const blockBorderColor = json.at("blockBorderColor").get<Color>();
        Float const blockBorderWidth = json.at("blockBorderWidth").get<Float>();
        Color const foundationHatchColor = json.at("foundationHatchColor").get<Color>();
        Float const foundationHatchWidth = json.at("foundationHatchWidth").get<Float>();
        return BlockTypePhase{ blockBorderColor, blockBorderWidth, foundationHatchColor, foundationHatchWidth };
    }
};
