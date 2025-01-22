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
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvasContext.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgRect.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/Config.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class SvgWorldBox {
    public:
        using Float = typename G::RealRep;
        using SyncWorld = typename G::Worlds::SyncWorld;

        using BlockIndex = typename SyncWorld::BlockIndex;
        using GridCoord = typename SyncWorld::BlockIndex::Coord;
        using SvgCanvasContext = detail::SvgCanvasContext<G>;
        using SvgRect = detail::SvgRect<Float>;

        [[nodiscard]]
        explicit SvgWorldBox(SvgCanvasContext const& ctx)
            : SvgWorldBox{ initData(ctx) }
        {}

        [[nodiscard]]
        SvgRect blockCoordinates(BlockIndex const& index) const {
            Float const x = svgBlockWidth_ * (0.5f + Float(index.x - xMin_));
            Float const y = svgBlockHeight_ * (0.5f + Float(yMax_ - index.y));
            return SvgRect{ x, y, svgBlockWidth_, svgBlockHeight_ };
        }

        [[nodiscard]]
        SvgRect boxCoordinates() const {
            return SvgRect{ 0.f, 0.f, boxWidth_, boxHeight_ };
        }
    private:
        struct Init {
            GridCoord xMin;
            GridCoord yMax;
            Float svgBlockHeight;
            Float svgBlockWidth;
            Float boxHeight;
            Float boxWidth;
        };

        [[nodiscard]]
        static Init initData(SvgCanvasContext const& ctx) {
            using Limits = std::numeric_limits<GridCoord>;
            auto const& sWorld = ctx.world().syncWorld();
            GridCoord xMax = Limits::min();
            GridCoord xMin = Limits::max();
            GridCoord yMax = Limits::min();
            GridCoord yMin = Limits::max();
            if (sWorld.blocks().size() == 0) {
                xMax = 0;
                xMin = 0;
                yMax = 0;
                yMin = 0;
            }
            else {
                for (auto const& block : sWorld.blocks()) {
                    auto const& id = block.index();
                    xMax = std::max(xMax, id.x);
                    xMin = std::min(xMin, id.x);
                    yMax = std::max(yMax, id.y);
                    yMin = std::min(yMin, id.y);
                }
            }
            Float const svgBlockHeight = ctx.svgBlockHeight();
            Float const svgBlockWidth = ctx.svgBlockWidth();
            return Init{
                .xMin = xMin,
                .yMax = yMax,
                .svgBlockHeight = svgBlockHeight,
                .svgBlockWidth = svgBlockWidth,
                .boxHeight = svgBlockHeight * Float(2 + yMax - yMin),
                .boxWidth = svgBlockWidth * Float(2 + xMax - xMin),
            };
        }

        [[nodiscard]]
        explicit SvgWorldBox(Init const& init)
            : xMin_{ init.xMin }
            , yMax_{ init.yMax }
            , svgBlockHeight_{ init.svgBlockHeight }
            , svgBlockWidth_{ init.svgBlockWidth }
            , boxHeight_{ init.boxHeight }
            , boxWidth_{ init.boxWidth }
        {}

        GridCoord xMin_;
        GridCoord yMax_;
        Float svgBlockHeight_;
        Float svgBlockWidth_;
        Float boxHeight_;
        Float boxWidth_;
    };
}
