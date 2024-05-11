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

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/RenderContext.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class ContactStressPhase : public Phase<G> {
    private:
        template<auto unit>
        using Real = typename G::template Real<unit>;

        using NormalizedVector3 = typename G::NormalizedVector3;

        static constexpr auto u = G::units();
        static constexpr auto rt = G::libConfig().realTraits;
    public:
        using Float = typename G::RealRep;
        using Color = jsonGustave::Color<Float>;
        using RenderContext = svgRenderer::RenderContext<G>;

        struct ColorPoint {
            Float stressValue;
            Color colorBefore;
            Color colorAfter;
        };

        [[nodiscard]]
        ContactStressPhase()
            : strokeWidth_{ 1.f }
            , strokeColor_{ 1.f, 1.f , 1.f }
            , colors_{ defaultColors() }
        {}

        [[nodiscard]]
        explicit ContactStressPhase(Float strokeWidth, Color const& strokeColor, std::vector<ColorPoint> colorPalette)
            : strokeWidth_{ strokeWidth }
            , strokeColor_{ strokeColor }
            , colors_{ std::move(colorPalette) }
        {
            throwIfColorsInvalid();
        }

        virtual void run(RenderContext& ctx) const override {
            auto const mForce = maxForce(ctx);
            auto const g = NormalizedVector3{ ctx.world().syncWorld().g() };
            ctx.startGroup({ {"stroke", strokeColor_.svgCode() },{"stroke-width", strokeWidth_} });
            for (auto const& contact : ctx.world().syncWorld().links()) {
                auto const stressRatio = contact.stressRatio();
                auto const stressFactor = stressRatio.maxCoord();
                auto const color = colorOfStress(stressFactor.value());
                auto const forceVector = contact.forceVector();
                auto const lengthFactor = (forceVector.norm() / mForce).value();
                if (forceVector.dot(g) > 0.f * u.force) {
                    ctx.drawContactArrow(contact, lengthFactor, { {"fill",color} });
                }
                else {
                    ctx.drawContactArrow(contact.opposite(), lengthFactor, { {"fill",color} });
                }
            }
            ctx.endGroup();
        }
    private:
        [[nodiscard]]
        static std::vector<ColorPoint> defaultColors() {
            auto const green = Color{ 0.f, 1.f, 0.f };
            auto const yellow = Color{ 1.f, 1.f, 0.f };
            auto const orange = Color{ 1.f, 0.5f, 0.f };
            auto const red = Color{ 1.f, 0.f, 0.f };
            auto const brown = Color{ 0.1f, 0.f, 0.f };
            return {
                { 0.f, green, green },
                { 0.5f, yellow, yellow },
                { 1.f, orange, red },
                { 4.f, brown, brown },
            };
        }

        [[nodiscard]]
        std::string colorOfStress(Float stress) const {
            auto pointIt = colors_.begin();
            if (stress <= pointIt->stressValue) {
                return pointIt->colorBefore.svgCode();
            }
            auto nextIt = pointIt + 1;
            while (nextIt != colors_.end()) {
                if (stress <= nextIt->stressValue) {
                    auto relDelta = (stress - pointIt->stressValue) / (nextIt->stressValue - pointIt->stressValue);
                    auto color = (1.f - relDelta) * pointIt->colorAfter + relDelta * nextIt->colorBefore;
                    return color.svgCode();
                }
                ++pointIt;
                ++nextIt;
            }
            return pointIt->colorAfter.svgCode();
        }

        [[nodiscard]]
        static Real<u.force> maxForce(RenderContext& ctx) {
            auto result = Real<u.force>::zero();
            for (auto const& contact : ctx.world().syncWorld().links()) {
                result = rt.max(result, contact.forceVector().norm());
            }
            return result;
        }

        void throwIfColorsInvalid() const {
            if (colors_.size() == 0) {
                throw std::invalid_argument("Invalid colorPalette: cannot be empty.");
            }
            for (std::size_t i = 0; 1 + i < colors_.size(); ++i) {
                auto const stress1 = colors_[i].stressValue;
                auto const stress2 = colors_[i + 1].stressValue;
                if (stress1 >= stress2) {
                    std::stringstream msg;
                    msg << "Invalid colorPalette: must be in sorted in strictly increasing values of '.stressValue' ( colors[";
                    msg << i << "] = " << stress1 << "; colors[" << i + 1 << "] = " << stress2 << ").";
                    throw std::invalid_argument(msg.str());
                }
            }
        }

        Float strokeWidth_;
        Color strokeColor_;
        std::vector<ColorPoint> colors_;
    };
}
