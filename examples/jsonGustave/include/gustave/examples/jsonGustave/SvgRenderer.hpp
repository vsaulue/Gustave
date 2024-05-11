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

#include <concepts>
#include <memory>
#include <ostream>
#include <utility>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/BlockTypePhase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/ContactStressPhase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/WorldFramePhase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/RenderContext.hpp>
#include <gustave/examples/jsonGustave/JsonWorld.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::examples::jsonGustave {
    template<core::cGustave G>
    class SvgRenderer {
    private:
        using Float = typename G::RealRep;
        using RenderContext = svgRenderer::RenderContext<G>;
    public:
        using Config = svgRenderer::Config<Float>;
        using JsonWorld = jsonGustave::JsonWorld<G>;
        using Phase = svgRenderer::phases::Phase<G>;

        [[nodiscard]]
        SvgRenderer() = default;

        [[nodiscard]]
        explicit SvgRenderer(Config const& config)
            : config_{ config }
            , phases_{}
        {}

        struct Phases {
            using BlockTypePhase = svgRenderer::phases::BlockTypePhase<G>;
            using ContactStressPhase = svgRenderer::phases::ContactStressPhase<G>;
            using WorldFramePhase = svgRenderer::phases::WorldFramePhase<G>;
        };

        void addPhase(std::derived_from<Phase> auto const& phase) {
            using DerivedPhase = decltype(meta::value(phase));
            phases_.push_back(std::make_unique<DerivedPhase>(phase));
        }

        void run(JsonWorld const& world, std::ostream& output) const {
            RenderContext ctx{ world, output, config_ };
            for (auto const& phase : phases_) {
                phase->run(ctx);
            }
            ctx.finalize();
        }
    private:
        Config config_;
        std::vector<std::unique_ptr<Phase>> phases_;
    };
}