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
#include <gustave/examples/jsonGustave/svgRenderer/detail/LegendColorScale.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/detail/LegendContactLength.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/ColorPoint.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/ColorScale.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::phases {
    template<core::cGustave G>
    class ContactStressPhase : public Phase<G> {
    private:
        template<auto unit>
        using Real = typename G::template Real<unit>;

        using NormalizedVector3 = typename G::NormalizedVector3;

        using LegendColorScale = detail::LegendColorScale<G>;
        using LegendContactLength = detail::LegendContactLength<G>;

        static constexpr auto u = G::units();
        static constexpr auto rt = G::libConfig().realTraits;
    public:
        using Float = typename G::RealRep;

        using Color = jsonGustave::Color<Float>;
        using ColorPoint = svgRenderer::ColorPoint<Float>;
        using ColorScale = svgRenderer::ColorScale<Float>;
        using Config = typename Phase<G>::Config;
        using JsonWorld = typename Phase<G>::JsonWorld;
        using PhaseContext = typename Phase<G>::PhaseContext;
        using SvgCanvasContext = typename Phase<G>::SvgCanvasContext;
        using SvgDims = typename Phase<G>::SvgDims;
        using SvgPhaseCanvas = typename Phase<G>::SvgPhaseCanvas;

        class ContactStressPhaseContext : public PhaseContext {
        public:
            [[nodiscard]]
            explicit ContactStressPhaseContext(SvgCanvasContext const& ctx, ContactStressPhase const& phase)
                : PhaseContext{ ctx }
                , phase_{ phase }
                , maxForce_{ computeMaxForce(ctx) }
                , legendScale_{ phase.stressColors_, ctx, std::string{ legendColorTitle() }, 0.f, 0.f }
                , legendLength_{ initLegendLength(ctx, phase, maxForce_, legendScale_.yMax() + ctx.config().legendSpace()) }
            {
                this->setLegendDims(computeLegendDims());
            }

            void render(SvgPhaseCanvas& canvas) const override {
                renderWorld(canvas);
                renderLegend(canvas);
            }
        private:
            [[nodiscard]]
            SvgDims computeLegendDims() const {
                auto const& scaleDims = legendScale_.dims();
                auto const& lengthDims = legendLength_.dims();
                Float const height = this->config().legendSpace() + scaleDims.height() + lengthDims.height();
                return { std::max(scaleDims.width(), lengthDims.width()), height };
            }

            [[nodiscard]]
            static Real<u.force> computeMaxForce(SvgCanvasContext const& ctx) {
                auto result = Real<u.force>::zero();
                for (auto const& contact : ctx.world().syncWorld().links()) {
                    result = rt.max(result, contact.forceVector().norm());
                }
                return result;
            }

            [[nodiscard]]
            static LegendContactLength initLegendLength(SvgCanvasContext const& ctx, ContactStressPhase const& phase, Real<u.force> maxForce, Float yMin) {
                std::stringstream minCaption;
                minCaption << Real<u.force>::zero();
                std::stringstream maxCaption;
                maxCaption << maxForce;
                return LegendContactLength{
                    ctx,
                    phase.strokeWidth_,
                    phase.strokeColor_,
                    phase.stressColors_.colorAt(0.f),
                    std::string{ legendLengthTitle() },
                    minCaption.str(),
                    maxCaption.str(),
                    0.f,
                    yMin,
                };
            }

            [[nodiscard]]
            static std::string_view legendColorTitle() {
                return "Contact arrow color (max stress ratio):";
            }

            [[nodiscard]]
            static std::string_view legendLengthTitle() {
                return "Contact arrow length (weight transfert):";
            }

            void renderLegend(SvgPhaseCanvas& canvas) const {
                legendScale_.render(canvas);
                legendLength_.render(canvas);
            }

            void renderWorld(SvgPhaseCanvas& canvas) const {
                auto const g = NormalizedVector3{ this->syncWorld().g() };
                canvas.startGroup({ {"stroke", phase_.strokeColor_.svgCode() },{"stroke-width", phase_.strokeWidth_} });
                for (auto const& contact : this->syncWorld().links()) {
                    auto const stressFactor = contact.stressRatio().maxCoord();
                    auto const color = phase_.stressColors_.colorAt(stressFactor.value()).svgCode();
                    auto const forceVector = contact.forceVector();
                    auto const lengthFactor = (forceVector.norm() / maxForce_).value();
                    if (forceVector.dot(g) > 0.f * u.force) {
                        canvas.drawWorldContactArrow(contact, lengthFactor, { {"fill",color} });
                    } else {
                        canvas.drawWorldContactArrow(contact.opposite(), lengthFactor, { {"fill",color} });
                    }
                }
                canvas.endGroup();
            }

            ContactStressPhase const& phase_;
            Real<u.force> maxForce_;
            LegendColorScale legendScale_;
            LegendContactLength legendLength_;
        };

        [[nodiscard]]
        ContactStressPhase()
            : strokeWidth_{ 1.f }
            , strokeColor_{ 1.f, 1.f , 1.f }
            , stressColors_{ ColorScale::defaultStressScale() }
        {}

        [[nodiscard]]
        explicit ContactStressPhase(Float strokeWidth, Color const& strokeColor, ColorScale stressColors)
            : strokeWidth_{ strokeWidth }
            , strokeColor_{ strokeColor }
            , stressColors_{ std::move(stressColors) }
        {}

        [[nodiscard]]
        std::unique_ptr<PhaseContext> makeContext(SvgCanvasContext const& canvasCtx) const override {
            return std::make_unique<ContactStressPhaseContext>(canvasCtx, *this);
        }
    private:
        Float strokeWidth_;
        Color strokeColor_;
        ColorScale stressColors_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::phases::ContactStressPhase<G>> {
    using ContactStressPhase = gustave::examples::jsonGustave::svgRenderer::phases::ContactStressPhase<G>;

    using Color = typename ContactStressPhase::Color;
    using ColorScale = typename ContactStressPhase::ColorScale;
    using Float = typename ContactStressPhase::Float;

    [[nodiscard]]
    static ContactStressPhase from_json(nlohmann::json const& json) {
        Float const strokeWidth = json.at("arrowBorderWidth").get<Float>();
        Color const strokeColor = json.at("arrowBorderColor").get<Color>();
        auto colorScale = json.at("stressColorScale").get<ColorScale>();
        return ContactStressPhase{ strokeWidth, strokeColor, std::move(colorScale) };
    }
};
