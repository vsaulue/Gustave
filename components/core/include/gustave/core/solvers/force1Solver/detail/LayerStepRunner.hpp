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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>

#include <gustave/core/solvers/force1Solver/detail/SolverRunContext.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class LayerStepRunner {
    private:
        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        static constexpr Real<u.one> targetErrorFactor = 0.75f;

        using SolverRunContext = detail::SolverRunContext<libCfg>;

        using LayerStructure = typename SolverRunContext::LayerStructure;
        using Layer = typename LayerStructure::Layer;

        [[nodiscard]]
        explicit LayerStepRunner(SolverRunContext& ctx)
            : ctx_{ ctx }
        {}

        void runStep() {
            Real<u.potential> cumulatedOffset = 0.f * u.potential;
            for (auto const& layer : ctx_.lStructure.layers()) {
                Real<u.potential> const layerOffset = findBalanceOffset(layer);
                cumulatedOffset += layerOffset;
                for (auto nodeIndex : layer.nodeIndices) {
                    ctx_.nextPotentials[nodeIndex] = ctx_.potentials[nodeIndex] + cumulatedOffset;
                }
            }
        }
    private:
        struct LayerStepPoint {
            Real<u.potential> offset;
            Real<u.force> force;
            Real<u.conductivity> derivative;

            [[nodiscard]]
            Real<u.potential> nextOffset() const {
                return offset - force / derivative;
            }
        };

        [[nodiscard]]
        LayerStepPoint pointAt(Layer const& layer, Real<u.potential> const offset) const {
            Real<u.force> force = layer.weight;
            Real<u.conductivity> derivative = 0.f * u.conductivity;
            for (auto const& contact : layer.lowContacts) {
                Real<u.potential> const localPotential = offset + ctx_.potentials[contact.localIndex()];
                Real<u.potential> const otherPotential = ctx_.potentials[contact.otherIndex()];
                auto const forceStats = contact.forceStats(localPotential, otherPotential);
                force += forceStats.force();
                derivative += forceStats.derivative();
            }
            return { offset, force, derivative };
        }

        [[nodiscard]]
        Real<u.potential> findBalanceOffset(Layer const& layer) const {
            Real<u.force> const maxForceError = targetErrorFactor * ctx_.config().targetMaxError() * layer.weight;
            LayerStepPoint curPoint = pointAt(layer, 0.f * u.potential);
            LayerStepPoint nextPoint = pointAt(layer, curPoint.nextOffset());
            if (rt.abs(nextPoint.force) <= maxForceError) {
                return nextPoint.offset;
            }
            bool const startSignBit = rt.signBit(curPoint.force);
            while (rt.signBit(nextPoint.force) == startSignBit) {
                curPoint = nextPoint;
                nextPoint = pointAt(layer, curPoint.nextOffset());
                assert(curPoint.offset != nextPoint.offset); // float resolution too low ?
                if (rt.abs(nextPoint.force) <= maxForceError) {
                    return nextPoint.offset;
                }
            }
            do {
                Real<u.resistance> const invDerivative = (nextPoint.offset - curPoint.offset) / (nextPoint.force - curPoint.force);
                LayerStepPoint const midPoint = pointAt(layer, curPoint.offset - invDerivative * curPoint.force);
                assert(midPoint.offset != curPoint.offset);// float resolution too low ?
                assert(midPoint.offset != nextPoint.offset); // float resolution too low ?
                if (rt.abs(midPoint.force) <= maxForceError) {
                    return midPoint.offset;
                }
                if (rt.signBit(midPoint.offset) == startSignBit) {
                    curPoint = midPoint;
                } else {
                    nextPoint = midPoint;
                }
            } while (true);
        }

        SolverRunContext& ctx_;
    };
}
