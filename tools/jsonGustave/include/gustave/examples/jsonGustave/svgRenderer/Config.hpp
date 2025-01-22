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

#include <stdexcept>
#include <sstream>
#include <string_view>

#include <gustave/cfg/cRealRep.hpp>

namespace gustave::examples::jsonGustave::svgRenderer {
    template<cfg::cRealRep Float_>
    class Config {
    public:
        using Float = Float_;

        using Color = jsonGustave::Color<Float>;

        [[nodiscard]]
        Config() = default;

        [[nodiscard]]
        Float arrowLineFactor() const {
            return arrowLineFactor_;
        }

        [[nodiscard]]
        Float arrowTriangleFactor() const {
            return arrowTriangleFactor_;
        }

        [[nodiscard]]
        Float legendColorScaleRes() const {
            return legendColorScaleRes_;
        }

        [[nodiscard]]
        Float legendColorScaleWidth() const {
            return legendColorScaleWidth_;
        }

        [[nodiscard]]
        Float legendSpace() const {
            return legendSpace_;
        }

        [[nodiscard]]
        Color legendTextColor() const {
            return legendTextColor_;
        }

        [[nodiscard]]
        Float legendTextSize() const {
            return legendTextSize_;
        }

        [[nodiscard]]
        Float legendTitleSize() const {
            return legendTitleSize_;
        }

        [[nodiscard]]
        Float spaceRes() const {
            return spaceRes_;
        }

        Config& setArrowLineFactor(Float value) {
            if (value <= 0.f || value > 1.f) {
                throw invalidFactorError("arrowLineFactor", value);
            }
            arrowLineFactor_ = value;
            return *this;
        }

        Config& setArrowTriangleFactor(Float value) {
            if (value <= 0.f || value > 1.f) {
                throw invalidFactorError("arrowTriangleFactor", value);
            }
            arrowTriangleFactor_ = value;
            return *this;
        }

        Config& setLegendColorScaleRes(Float value) {
            if (value <= 0.f) {
                throw invalidStrictPositiveError("legendColorScaleRes", value);
            }
            legendColorScaleRes_ = value;
            return *this;
        }

        Config& setLegendColorScaleWidth(Float value) {
            if (value < 0.f) {
                throw invalidPositiveError("legendColorScaleWidth", value);
            }
            legendColorScaleWidth_ = value;
            return *this;
        }

        Config& setLegendSpace(Float value) {
            if (value < 0.f) {
                throw invalidPositiveError("legendSpace", value);
            }
            legendSpace_ = value;
            return *this;
        }

        Config& setLegendTextColor(Color const& value) {
            legendTextColor_ = value;
            return *this;
        }

        Config& setLegendTextSize(Float value) {
            if (value < 0.f) {
                throw invalidPositiveError("legendTextSize", value);
            }
            legendTextSize_ = value;
            return *this;
        }

        Config& setLegendTitleSize(Float value) {
            if (value < 0.f) {
                throw invalidPositiveError("legendTitleSize", value);
            }
            legendTitleSize_ = value;
            return *this;
        }

        Config& setSpaceRes(Float value) {
            if (value <= 0.f) {
                throw invalidStrictPositiveError("spaceRes", value);
            }
            spaceRes_ = value;
            return *this;
        }
    private:
        [[nodiscard]]
        static std::invalid_argument invalidFactorError(std::string_view factorName, Float value) {
            std::stringstream msg;
            msg << "Invalid " << factorName << ": must be between 0 and 1 (passed: " << value << ").";
            return std::invalid_argument(msg.str());
        }

        [[nodiscard]]
        static std::invalid_argument invalidStrictPositiveError(std::string_view fieldName, Float value) {
            std::stringstream msg;
            msg << "Invalid " << fieldName << ": it must be strictly positive (passed: " << value << ").";
            throw std::invalid_argument(msg.str());
        }

        [[nodiscard]]
        static std::invalid_argument invalidPositiveError(std::string_view fieldName, Float value) {
            std::stringstream msg;
            msg << "Invalid " << fieldName << ": it must be positive (passed: " << value << ").";
            throw std::invalid_argument(msg.str());
        }

        Color legendTextColor_ = { 0.f, 0.f, 0.f };
        Float arrowLineFactor_ = 0.5f;
        Float arrowTriangleFactor_ = 0.1875f;
        Float legendColorScaleRes_ = 32.f;
        Float legendColorScaleWidth_ = 32.f;
        Float legendSpace_ = 8.f;
        Float legendTextSize_ = 12.f;
        Float legendTitleSize_ = 24.f;
        Float spaceRes_ = 32.f;
    };
}

template<gustave::cfg::cRealRep Float>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::Config<Float>> {
    using Config = gustave::examples::jsonGustave::svgRenderer::Config<Float>;

    using Color = typename Config::Color;

    static void from_json(nlohmann::json const& json, Config& config) {
        config.setArrowLineFactor(json.at("arrowLineFactor").get<Float>());
        config.setArrowTriangleFactor(json.at("arrowTriangleFactor").get<Float>());
        config.setLegendColorScaleRes(json.at("legendColorScaleRes").get<Float>());
        config.setLegendColorScaleWidth(json.at("legendColorScaleWidth").get<Float>());
        config.setLegendSpace(json.at("legendSpace").get<Float>());
        config.setLegendTextColor(json.at("legendTextColor").get<Color>());
        config.setLegendTextSize(json.at("legendTextSize").get<Float>());
        config.setLegendTitleSize(json.at("legendTitleSize").get<Float>());
        config.setSpaceRes(json.at("spaceResolution").get<Float>());
    }
};
