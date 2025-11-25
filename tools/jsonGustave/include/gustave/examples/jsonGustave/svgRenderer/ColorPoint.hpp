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

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer {
    template<cfg::cRealRep Float_>
    struct ColorPoint {
        using Color = jsonGustave::Color<Float_>;
        using Float = Float_;

        Float index;
        Color colorBefore;
        Color colorAfter;
    };
}

template<gustave::cfg::cRealRep Float>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::ColorPoint<Float>> {
    using ColorPoint = gustave::examples::jsonGustave::svgRenderer::ColorPoint<Float>;

    using Color = ColorPoint::Color;

    [[nodiscard]]
    static ColorPoint from_json(nlohmann::json const& json) {
        Float const index = json.at("index").get<Float>();
        Color const colorBefore = json.at("colorBefore").get<Color>();
        Color const colorAfter = json.at("colorAfter").get<Color>();
        return ColorPoint{ index, colorBefore, colorAfter };
    }
};
