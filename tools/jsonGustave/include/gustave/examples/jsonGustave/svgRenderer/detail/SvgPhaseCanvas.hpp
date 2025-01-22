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

#include <svgwrite/writer.hpp>

#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgCanvas.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    class SvgPhaseCanvas {
    public:
        using Float = typename G::RealRep;
        using SvgCanvas = detail::SvgCanvas<G>;
        using SyncWorld = typename G::Worlds::SyncWorld;

        using Attrs = typename SvgCanvas::Attrs;
        using BlockReference = typename SyncWorld::BlockReference;
        using ContactReference = typename SyncWorld::ContactReference;
        using SvgLinearGradient = typename SvgCanvas::SvgLinearGradient;
        using SvgRect = typename SvgCanvas::SvgRect;

        [[nodiscard]]
        explicit SvgPhaseCanvas(SvgCanvas& canvas, Float xLegendOffset, Float yLegendOffset)
            : canvas_{ canvas }
            , xLegendOffset_{ xLegendOffset }
            , yLegendOffset_{ yLegendOffset }
        {}

        [[nodiscard]]
        std::string defLinearGradient(SvgLinearGradient const& gradient) {
            return canvas_.defLinearGradient(gradient);
        }

        void drawLegendBlock(Float xMin, Float yMin, Attrs attrs) {
            canvas_.drawLegendBlock(xMin + xLegendOffset_, yMin + yLegendOffset_, attrs);
        }

        void drawLegendContactArrow(Float xMin, Float yMin, Float lengthFactor, Attrs attrs) {
            canvas_.drawLegendContactArrow(xMin + xLegendOffset_, yMin + yLegendOffset_, lengthFactor, attrs);
        }

        void drawLegendLine(Float x1, Float y1, Float x2, Float y2, Attrs attrs) {
            canvas_.drawLegendLine(x1 + xLegendOffset_, y1 + yLegendOffset_, x2 + xLegendOffset_, y2 + yLegendOffset_, attrs);
        }

        void drawLegendRect(SvgRect const& rect, Attrs attrs) {
            auto const absRect = SvgRect{ rect.xMin() + xLegendOffset_, rect.yMin() + yLegendOffset_, rect.width(), rect.height() };
            canvas_.drawLegendRect(absRect, attrs);
        }

        void drawLegendText(Float xMin, Float yMin, std::string_view text, Attrs attrs) {
            canvas_.drawLegendText(xMin + xLegendOffset_, yMin + yLegendOffset_, text, attrs);
        }

        void drawWorldBlock(BlockReference const& block, Attrs attrs) {
            canvas_.drawWorldBlock(block, attrs);
        }

        void drawWorldContactArrow(ContactReference const& contact, Float lengthRatio, Attrs attrs) {
            canvas_.drawWorldContactArrow(contact, lengthRatio, attrs);
        }

        void drawWorldFrame(Attrs attrs) {
            canvas_.drawWorldFrame(attrs);
        }

        void endGroup() {
            canvas_.endGroup();
        }

        void hatchLegendBlock(Float xMin, Float yMax, Attrs attrs) {
            canvas_.hatchLegendBlock(xMin + xLegendOffset_, yMax + yLegendOffset_, attrs);
        }

        void hatchWorldBlock(BlockReference const& block, Attrs attrs) {
            canvas_.hatchWorldBlock(block, attrs);
        }

        void startGroup(Attrs attrs) {
            canvas_.startGroup(attrs);
        }
    private:
        SvgCanvas& canvas_;
        Float xLegendOffset_;
        Float yLegendOffset_;
    };
}
