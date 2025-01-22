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
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class WorldFramePhase : public Phase<G> {
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using Config = typename Phase<G>::Config;
        using JsonWorld = typename Phase<G>::JsonWorld;
        using SvgCanvasContext = typename Phase<G>::SvgCanvasContext;
        using SvgPhaseCanvas = typename Phase<G>::SvgPhaseCanvas;
        using PhaseContext = typename Phase<G>::PhaseContext;

        class WorldFramePhaseContext : public PhaseContext {
        public:
            [[nodiscard]]
            explicit WorldFramePhaseContext(SvgCanvasContext const& ctx, WorldFramePhase const& phase)
                : PhaseContext{ ctx }
                , phase_{ phase }
            {}

            void render(SvgPhaseCanvas& canvas) const override {
                canvas.drawWorldFrame({ {"fill-opacity",0.f}, {"stroke", phase_.frameColor_.svgCode()},{"stroke-width", phase_.frameWidth_} });
            }
        private:
            WorldFramePhase const& phase_;
        };

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
                throw Phase<G>::invalidWidthError("frameWidth", frameWidth_);
            }
        }

        [[nodiscard]]
        std::unique_ptr<PhaseContext> makeContext(SvgCanvasContext const& canvasCtx) const override {
            return std::make_unique<WorldFramePhaseContext>(canvasCtx, *this);
        }
    private:
        Color frameColor_;
        Float frameWidth_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::phases::WorldFramePhase<G>> {
    using WorldFramePhase = gustave::examples::jsonGustave::svgRenderer::phases::WorldFramePhase<G>;

    using Color = typename WorldFramePhase::Color;
    using Float = typename WorldFramePhase::Float;

    [[nodiscard]]
    static WorldFramePhase from_json(nlohmann::json const& json) {
        Color const frameColor = json.at("frameColor").get<Color>();
        Float const frameWidth = json.at("frameWidth").get<Float>();
        return WorldFramePhase{ frameColor, frameWidth };
    }
};
