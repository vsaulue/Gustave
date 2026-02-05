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

#include <string>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/LegendBlockHatch.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/LegendColorScale.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/ColorScale.hpp>
#include <gustave/examples/jsonGustave/StressCoord.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class BlockStressPhase : public Phase<G> {
    public:
        using Float = G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using Config = Phase<G>::Config;
        using ColorScale = svgRenderer::ColorScale<Float>;
        using JsonWorld = Phase<G>::JsonWorld;
        using PhaseContext = Phase<G>::PhaseContext;
        using SvgCanvasContext = Phase<G>::SvgCanvasContext;
        using SvgDims = Phase<G>::SvgDims;
        using SvgPhaseCanvas = Phase<G>::SvgPhaseCanvas;
        using StressCoord = jsonGustave::StressCoord;
    private:
        using LegendBlockHatch = detail::LegendBlockHatch<G>;
        using LegendColorScale = detail::LegendColorScale<G>;
    public:

        class BlockStressPhaseContext : public PhaseContext {
        public:
            [[nodiscard]]
            explicit BlockStressPhaseContext(SvgCanvasContext const& ctx, BlockStressPhase const& phase)
                : PhaseContext{ ctx }
                , phase_{ phase }
                , legendColor_{ phase.stressColors_, ctx, legendColorTitle(phase), 0.f, 0.f}
                , legendHatch_{ ctx, phase.foundationHatchColor_, phase.foundationHatchWidth_, 0.f, legendColor_.yMax() + ctx.config().legendSpace() }
            {
                this->setLegendDims(generateLegendDims());
            }

        protected:
            void renderLegend(SvgPhaseCanvas& canvas) const override {
                legendColor_.render(canvas);
                legendHatch_.render(canvas);
            }

            void renderWorld(SvgPhaseCanvas& canvas) const override {
                canvas.startGroup({ {"stroke", phase_.blockBorderColor_.svgCode()}, {"stroke-width", phase_.blockBorderWidth_} });
                auto const hatchColorCode = phase_.foundationHatchColor_.svgCode();
                for (auto const& block : this->syncWorld().blocks()) {
                    auto const stress = phase_.stressCoord_.extract(block.stressRatio()).value();
                    auto const svgColor = phase_.stressColors_.colorAt(stress).svgCode();
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
                SvgDims const colorDims = legendColor_.dims();
                SvgDims const hatchDims = legendHatch_.dims();
                Float const width = std::max(colorDims.width(), hatchDims.width());
                Float const height = colorDims.height() + hatchDims.height() + this->config().legendSpace();
                return { width, height };
            }

            [[nodiscard]]
            static std::string legendColorTitle(BlockStressPhase const& phase) {
                switch (phase.stressCoord_.id()) {
                case StressCoord::Id::compression:
                    return "Block color (compression stress ratio):";
                case StressCoord::Id::max:
                    return "Block color (max stress ratio):";
                case StressCoord::Id::shear:
                    return "Block color (shear stress ratio):";
                case StressCoord::Id::tensile:
                    return "Block color (tensile stress ratio):";
                }
                throw phase.stressCoord_.invalidError();
            }

            BlockStressPhase const& phase_;
            LegendColorScale legendColor_;
            LegendBlockHatch legendHatch_;
        };

        [[nodiscard]]
        BlockStressPhase()
            : blockBorderColor_{ 0.f, 0.f, 0.f }
            , foundationHatchColor_{ 0.f, 0.f, 0.f }
            , blockBorderWidth_{ 1.f }
            , foundationHatchWidth_{ 2.f }
            , stressCoord_{ StressCoord::Id::max }
            , stressColors_{ ColorScale::defaultStressScale() }
        {}

        [[nodiscard]]
        explicit BlockStressPhase(
            Color const& blockBorderColor,
            Float blockBorderWidth,
            Color const& foundationHatchColor,
            Float foundationHatchWidth,
            StressCoord stressCoord,
            ColorScale stressColors
        )
            : blockBorderColor_{ blockBorderColor }
            , foundationHatchColor_{ foundationHatchColor }
            , blockBorderWidth_{ blockBorderWidth }
            , foundationHatchWidth_{ foundationHatchWidth }
            , stressCoord_{ stressCoord }
            , stressColors_{ std::move(stressColors) }
        {}

        [[nodiscard]]
        std::unique_ptr<PhaseContext> makeContext(SvgCanvasContext const& canvasCtx) const override {
            return std::make_unique<BlockStressPhaseContext>(canvasCtx, *this);
        }
    private:
        Color blockBorderColor_;
        Color foundationHatchColor_;
        Float blockBorderWidth_;
        Float foundationHatchWidth_;
        StressCoord stressCoord_;
        ColorScale stressColors_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::phases::BlockStressPhase<G>> {
    using BlockStressPhase = gustave::examples::jsonGustave::svgRenderer::phases::BlockStressPhase<G>;

    using Color = BlockStressPhase::Color;
    using ColorScale = BlockStressPhase::ColorScale;
    using Float = BlockStressPhase::Float;
    using StressCoord = BlockStressPhase::StressCoord;

    [[nodiscard]]
    static BlockStressPhase from_json(nlohmann::json const& json) {
        Color const borderColor = json.at("blockBorderColor").get<Color>();
        Float const borderWidth = json.at("blockBorderWidth").get<Float>();
        Color const hatchColor = json.at("foundationHatchColor").get<Color>();
        Float const hatchWidth = json.at("foundationHatchWidth").get<Float>();
        StressCoord const stressCoord = json.at("stressCoord").get<StressCoord>();
        auto colorScale = json.at("stressColorScale").get<ColorScale>();
        return BlockStressPhase{ borderColor, borderWidth, hatchColor, hatchWidth, stressCoord, std::move(colorScale) };
    }
};
