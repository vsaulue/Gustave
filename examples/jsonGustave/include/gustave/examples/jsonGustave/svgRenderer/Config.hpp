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
#include <stdexcept>
#include <sstream>
#include <string_view>

namespace gustave::examples::jsonGustave::svgRenderer {
    template<std::floating_point Float_>
    class Config {
    public:
        using Float = Float_;

        [[nodiscard]]
        Config()
            : arrowLineFactor_{ 0.5f }
            , arrowTriangleFactor_{ 0.1875f }
            , spaceRes_{ 32.f }
        {}

        [[nodiscard]]
        Float arrowLineFactor() const {
            return arrowLineFactor_;
        }

        [[nodiscard]]
        Float arrowTriangleFactor() const {
            return arrowTriangleFactor_;
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

        Config& setSpaceRes(Float value) {
            if (value <= 0.f) {
                std::stringstream msg;
                msg << "Invalid spaceRes: it must be strictly positive (passed: " << spaceRes_ << ").";
                throw std::invalid_argument(msg.str());
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

        Float arrowLineFactor_ = 0.5f;
        Float arrowTriangleFactor_ = 0.1875f;
        Float spaceRes_;
    };
}
