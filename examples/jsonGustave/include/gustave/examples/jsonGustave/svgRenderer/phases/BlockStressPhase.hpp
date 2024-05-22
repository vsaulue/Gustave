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
#include <gustave/examples/jsonGustave/svgRenderer/RenderContext.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class BlockStressPhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;
        using RenderContext = svgRenderer::RenderContext<G>;

        using Color = jsonGustave::Color<Float>;
        using ColorScale = svgRenderer::ColorScale<Float>;

        [[nodiscard]]
        BlockStressPhase()
            : blockBorderColor_{ 0.f, 0.f, 0.f }
            , foundationHatchColor_{ 0.f, 0.f, 0.f }
            , blockBorderWidth_{ 1.f }
            , foundationHatchWidth_{ 2.f }
            , stressColors_{ ColorScale::defaultStressScale() }
        {}

        [[nodiscard]]
        explicit BlockStressPhase(
            Color const& blockBorderColor,
            Float blockBorderWidth,
            Color const& foundationHatchColor,
            Float foundationHatchWidth,
            ColorScale stressColors
        )
            : blockBorderColor_{ blockBorderColor }
            , foundationHatchColor_{ foundationHatchColor }
            , blockBorderWidth_{ blockBorderWidth }
            , foundationHatchWidth_{ foundationHatchWidth }
            , stressColors_{ std::move(stressColors) }
        {}

        virtual void run(RenderContext& ctx) const override {
            ctx.startGroup({ {"stroke", blockBorderColor_.svgCode()}, {"stroke-width", blockBorderWidth_} });
            auto const hatchColorCode = foundationHatchColor_.svgCode();
            for (auto const& block : ctx.world().syncWorld().blocks()) {
                auto const maxStress = block.stressRatio().maxCoord().value();
                auto const svgColor = stressColors_.colorAt(maxStress).svgCode();
                ctx.drawBlock(block, { {"fill", svgColor } });
                if (block.isFoundation()) {
                    ctx.hatchBlock(block, { {"stroke", hatchColorCode },{"stroke-width", foundationHatchWidth_} });
                }
            }
            ctx.endGroup();
        }
    private:
        Color blockBorderColor_;
        Color foundationHatchColor_;
        Float blockBorderWidth_;
        Float foundationHatchWidth_;
        ColorScale stressColors_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::phases::BlockStressPhase<G>> {
    using BlockStressPhase = gustave::examples::jsonGustave::svgRenderer::phases::BlockStressPhase<G>;

    using Color = typename BlockStressPhase::Color;
    using ColorScale = typename BlockStressPhase::ColorScale;
    using Float = typename BlockStressPhase::Float;

    [[nodiscard]]
    static BlockStressPhase from_json(nlohmann::json const& json) {
        Color const borderColor = json.at("blockBorderColor").get<Color>();
        Float const borderWidth = json.at("blockBorderWidth").get<Float>();
        Color const hatchColor = json.at("foundationHatchColor").get<Color>();
        Float const hatchWidth = json.at("foundationHatchWidth").get<Float>();
        auto colorScale = json.at("stressColorScale").get<ColorScale>();
        return BlockStressPhase{ borderColor, borderWidth, hatchColor, hatchWidth, std::move(colorScale) };
    }
};
