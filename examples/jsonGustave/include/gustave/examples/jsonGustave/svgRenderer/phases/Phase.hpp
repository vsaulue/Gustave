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

#include <stdexcept>
#include <string_view>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgPhaseCanvas.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/Config.hpp>
#include <gustave/examples/jsonGustave/JsonWorld.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class Phase {
    public:
        using Float = typename G::RealRep;

        using Config = svgRenderer::Config<Float>;
        using JsonWorld = jsonGustave::JsonWorld<G>;
        using SyncWorld = typename JsonWorld::SyncWorld;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgPhaseCanvas = detail::SvgPhaseCanvas<G>;

        class PhaseContext {
        public:
            [[nodiscard]]
            explicit PhaseContext(SvgCanvasContext const& ctx)
                : canvasCtx_{ ctx }
            {}

            virtual ~PhaseContext() = default;
            virtual void render(SvgPhaseCanvas& canvas) const = 0;
        protected:
            [[nodiscard]]
            JsonWorld const& jsonWorld() const {
                return canvasCtx_.world();
            }

            [[nodiscard]]
            SyncWorld const& syncWorld() const {
                return canvasCtx_.world().syncWorld();
            }

            SvgCanvasContext const& canvasCtx_;
        };

        virtual ~Phase() = default;

        [[nodiscard]]
        virtual std::unique_ptr<PhaseContext> makeContext(SvgCanvasContext const& canvasCtx) const = 0;
    protected:
        [[nodiscard]]
        static std::invalid_argument invalidWidthError(std::string_view fieldName, Float value) {
            std::stringstream msg;
            msg << "Invalid value for '" << fieldName << "': must be positive (passed: " << value << ").";
            return std::invalid_argument{ msg.str() };
        }
    };
}
