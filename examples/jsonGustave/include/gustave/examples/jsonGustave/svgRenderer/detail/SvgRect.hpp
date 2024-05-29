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

#include <cassert>

#include <gustave/cfg/cRealRep.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<cfg::cRealRep Float_>
    class SvgRect {
    public:
        using Float = Float_;

        [[nodiscard]]
        explicit SvgRect(Float xMin, Float yMin, Float width, Float height)
            : xMin_{ xMin }
            , yMin_{ yMin }
            , width_{ width }
            , height_{ height }
        {
            assert(width_ >= 0.f);
            assert(height_ >= 0.f);
        }

        [[nodiscard]]
        Float height() const {
            return height_;
        }

        [[nodiscard]]
        Float width() const {
            return width_;
        }

        [[nodiscard]]
        Float xMax() const {
            return xMin_ + width_;
        }

        [[nodiscard]]
        Float xMean() const {
            return xMin_ + width_ / 2;
        }

        [[nodiscard]]
        Float xMin() const {
            return xMin_;
        }

        [[nodiscard]]
        Float yMax() const {
            return yMin_ + height_;
        }

        [[nodiscard]]
        Float yMean() const {
            return yMin_ + height_ / 2;
        }

        [[nodiscard]]
        Float yMin() const {
            return yMin_;
        }
    private:
        Float xMin_;
        Float yMin_;
        Float width_;
        Float height_;
    };
}
