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
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class BlockTypePhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using RenderContext = svgRenderer::RenderContext<G>;

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
                throw invalidWidthError("blockBorderWidth", blockBorderWidth_);
            }
            if (foundationHatchWidth_ < 0.f) {
                throw invalidWidthError("foundationHatchWidth", foundationHatchWidth_);
            }
        }

        virtual void run(RenderContext& ctx) const override {
            ctx.startGroup({ {"stroke",blockBorderColor_.svgCode()}, {"stroke-width", blockBorderWidth_} });
            auto const hatchColorCode = foundationHatchColor_.svgCode();
            for (auto const& block : ctx.world().syncWorld().blocks()) {
                auto const svgColor = ctx.world().blockTypeOf().at(block.index())->color().svgCode();
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
    };
}
