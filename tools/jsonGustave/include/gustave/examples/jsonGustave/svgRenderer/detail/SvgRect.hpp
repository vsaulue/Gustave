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

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/SvgDims.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<cfg::cRealRep Float_>
    class SvgRect {
    public:
        using Float = Float_;

        using SvgDims = detail::SvgDims<Float>;

        [[nodiscard]]
        explicit SvgRect(Float xMin, Float yMin, Float width, Float height)
            : xMin_{ xMin }
            , yMin_{ yMin }
            , dims_{ width, height }
        {}

        [[nodiscard]]
        Float height() const {
            return dims_.height();
        }

        void setHeight(Float value) {
            dims_.setHeight(value);
        }

        void setWidth(Float value) {
            dims_.setWidth(value);
        }

        [[nodiscard]]
        Float width() const {
            return dims_.width();
        }

        [[nodiscard]]
        Float xMax() const {
            return xMin_ + dims_.width();
        }

        [[nodiscard]]
        Float xMean() const {
            return xMin_ + dims_.width() / 2;
        }

        [[nodiscard]]
        Float xMin() const {
            return xMin_;
        }

        [[nodiscard]]
        Float& xMin() {
            return xMin_;
        }

        [[nodiscard]]
        Float yMax() const {
            return yMin_ + dims_.height();
        }

        [[nodiscard]]
        Float yMean() const {
            return yMin_ + dims_.height() / 2;
        }

        [[nodiscard]]
        Float yMin() const {
            return yMin_;
        }

        [[nodiscard]]
        Float& yMin() {
            return yMin_;
        }
    private:
        Float xMin_;
        Float yMin_;
        SvgDims dims_;
    };
}
