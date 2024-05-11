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
#include <gustave/examples/jsonGustave/svgRenderer/RenderContext.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class WorldFramePhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;
        using Color = jsonGustave::Color<Float>;
        using RenderContext = svgRenderer::RenderContext<G>;

        [[nodiscard]]
        WorldFramePhase()
            : frameColor_{ 0.f, 0.f, 0.f }
            , frameWidth_{ 1.f }
        {}

        [[nodiscard]]
        explicit WorldFramePhase(Color const& frameColor, Float frameWidth)
            : frameColor_{ frameColor }
            , frameWidth_{ frameWidth }
        {
            if (frameWidth_ < 0.f) {
                throw invalidWidthError("frameWidth", frameWidth_);
            }
        }

        virtual void run(RenderContext& ctx) const override {
            ctx.drawWorldFrame({ {"fill-opacity",0.f}, {"stroke", frameColor_.svgCode()},{"stroke-width",frameWidth_} });
        }
    private:
        Color frameColor_;
        Float frameWidth_;
    };
}
