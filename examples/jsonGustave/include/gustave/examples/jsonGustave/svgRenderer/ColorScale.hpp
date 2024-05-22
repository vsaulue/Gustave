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
#include <utility>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/ColorPoint.hpp>

namespace gustave::examples::jsonGustave::svgRenderer {
    template<cfg::cRealRep Float_>
    class ColorScale {
    public:
        using ColorPoint = svgRenderer::ColorPoint<Float_>;
        using Float = Float_;

        using Color = typename ColorPoint::Color;

        [[nodiscard]]
        explicit ColorScale(std::vector<ColorPoint> colorPoints)
            : colorPoints_{ std::move(colorPoints) }
        {
            throwIfIndicesInvalid();
        }

        [[nodiscard]]
        Color colorAt(Float index) const {
            auto pointIt = colorPoints_.begin();
            if (index <= pointIt->index) {
                return pointIt->colorBefore;
            }
            auto nextIt = pointIt + 1;
            while (nextIt != colorPoints_.end()) {
                if (index <= nextIt->index) {
                    auto relDelta = (index - pointIt->index) / (nextIt->index - pointIt->index);
                    return (1.f - relDelta) * pointIt->colorAfter + relDelta * nextIt->colorBefore;
                }
                ++pointIt;
                ++nextIt;
            }
            return pointIt->colorAfter;
        }

        [[nodiscard]]
        static ColorScale defaultStressScale() {
            auto const green = Color{ 0.f, 1.f, 0.f };
            auto const yellow = Color{ 1.f, 1.f, 0.f };
            auto const orange = Color{ 1.f, 0.5f, 0.f };
            auto const red = Color{ 1.f, 0.f, 0.f };
            auto const brown = Color{ 0.1f, 0.f, 0.f };
            return ColorScale{ std::vector<ColorPoint>{
                { 0.f, green, green },
                { 0.5f, yellow, yellow },
                { 1.f, orange, red },
                { 4.f, brown, brown },
            } };
        }
    private:
        void throwIfIndicesInvalid() const {
            if (colorPoints_.size() == 0) {
                throw std::invalid_argument("Invalid colorPalette: cannot be empty.");
            }
            for (std::size_t i = 0; 1 + i < colorPoints_.size(); ++i) {
                auto const index1 = colorPoints_[i].index;
                auto const index2 = colorPoints_[i + 1].index;
                if (index1 >= index2) {
                    std::stringstream msg;
                    msg << "Invalid colorPalette: must be in sorted in strictly increasing values of '.index' ( colors[";
                    msg << i << "] = " << index1 << "; colors[" << i + 1 << "] = " << index2 << ").";
                    throw std::invalid_argument(msg.str());
                }
            }
        }

        std::vector<ColorPoint> colorPoints_;
    };
}

template<gustave::cfg::cRealRep Float>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::ColorScale<Float>> {
    using ColorScale = gustave::examples::jsonGustave::svgRenderer::ColorScale<Float>;

    using ColorPoint = typename ColorScale::ColorPoint;

    [[nodiscard]]
    static ColorScale from_json(nlohmann::json const& json) {
        return ColorScale{ json.get<std::vector<ColorPoint>>() };
    }
};
