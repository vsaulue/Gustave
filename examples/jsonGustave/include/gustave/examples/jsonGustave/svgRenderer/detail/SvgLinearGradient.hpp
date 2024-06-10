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

#include <span>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<cfg::cRealRep Float_>
    class SvgLinearGradient {
    public:
        using Float = Float_;

        using Color = jsonGustave::Color<Float>;

        struct Stop {
            Float offset;
            Color color;
        };

        [[nodiscard]]
        explicit SvgLinearGradient(Float x1, Float x2, Float y1, Float y2, std::span<Stop const> stops)
            : x1_{ x1 }
            , x2_{ x2 }
            , y1_{ y1 }
            , y2_{ y2 }
            , stops_{ stops }
        {}

        [[nodiscard]]
        Float x1() const {
            return x1_;
        }

        [[nodiscard]]
        Float x2() const {
            return x2_;
        }

        [[nodiscard]]
        Float y1() const {
            return y1_;
        }

        [[nodiscard]]
        Float y2() const {
            return y2_;
        }

        [[nodiscard]]
        std::span<Stop const> const& stops() const {
            return stops_;
        }
    private:
        Float x1_;
        Float x2_;
        Float y1_;
        Float y2_;
        std::span<Stop const> stops_;
    };
}
