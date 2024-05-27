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

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/ColorScale.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/SvgCanvas.hpp>
#include <gustave/examples/jsonGustave/StressCoord.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class BlockStressPhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using Config = typename Phase<G>::Config;
        using ColorScale = svgRenderer::ColorScale<Float>;
        using JsonWorld = typename Phase<G>::JsonWorld;
        using PhaseContext = typename Phase<G>::PhaseContext;
        using SvgCanvas = svgRenderer::SvgCanvas<G>;
        using StressCoord = jsonGustave::StressCoord;

        class BlockStressPhaseContext : public PhaseContext {
        public:
            [[nodiscard]]
            explicit BlockStressPhaseContext(Config const& config, JsonWorld const& world, BlockStressPhase const& phase)
                : PhaseContext{ config, world }
                , phase_{ phase }
            {}

            void render(SvgCanvas& canvas) const override {
                canvas.startGroup({ {"stroke", phase_.blockBorderColor_.svgCode()}, {"stroke-width", phase_.blockBorderWidth_} });
                auto const hatchColorCode = phase_.foundationHatchColor_.svgCode();
                for (auto const& block : this->world_.syncWorld().blocks()) {
                    auto const stress = phase_.stressCoord_.extract(block.stressRatio()).value();
                    auto const svgColor = phase_.stressColors_.colorAt(stress).svgCode();
                    canvas.drawBlock(block, { {"fill", svgColor } });
                    if (block.isFoundation()) {
                        canvas.hatchBlock(block, { {"stroke", hatchColorCode },{"stroke-width", phase_.foundationHatchWidth_}});
                    }
                }
                canvas.endGroup();
            }
        private:
            BlockStressPhase const& phase_;
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
        std::unique_ptr<PhaseContext> makeContext(Config const& config, JsonWorld const& world) const override {
            return std::make_unique<BlockStressPhaseContext>(config, world, *this);
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

    using Color = typename BlockStressPhase::Color;
    using ColorScale = typename BlockStressPhase::ColorScale;
    using Float = typename BlockStressPhase::Float;
    using StressCoord = typename BlockStressPhase::StressCoord;

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
