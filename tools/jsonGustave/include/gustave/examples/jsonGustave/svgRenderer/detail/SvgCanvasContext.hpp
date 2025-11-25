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

#include <string_view>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/Config.hpp>
#include <gustave/examples/jsonGustave/JsonWorld.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class SvgCanvasContext {
    public:
        using Float = G::RealRep;

        using World = jsonGustave::JsonWorld<G>;
        using Config = svgRenderer::Config<Float>;

        [[nodiscard]]
        explicit SvgCanvasContext(World const& world, Config const& config)
            : world_{ world }
            , config_{ config }
        {
            auto const& blockSize = world.syncWorld().scene().blockSize();
            svgBlockHeight_ = config.spaceRes() * blockSize.y().value();
            svgBlockWidth_ = config.spaceRes() * blockSize.x().value();
        }

        [[nodiscard]]
        Config const& config() const {
            return config_;
        }

        [[nodiscard]]
        Float svgBlockHeight() const {
            return svgBlockHeight_;
        }

        [[nodiscard]]
        Float svgBlockWidth() const {
            return svgBlockWidth_;
        }

        [[nodiscard]]
        Float textWidth(std::string_view text, Float fontSize) const {
            return 0.6f * fontSize * Float(text.size()); // horrible estimate.
        }

        [[nodiscard]]
        World const& world() const {
            return world_;
        }
    private:
        World const& world_;
        Config const& config_;
        Float svgBlockHeight_;
        Float svgBlockWidth_;
    };
}
