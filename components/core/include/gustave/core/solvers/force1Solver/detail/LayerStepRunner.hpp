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

#include <vector>

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
            auto const& layers = ctx_.lStructure.layers();
            auto& layerOffsets = ctx_.nextPotentials;
            assert(layerOffsets.size() >= layers.size());
            for (std::size_t layerId = 0; layerId < layers.size(); ++layerId) {
                auto const& layer = layers[layerId];
                if (layer.isFoundation()) {
                    layerOffsets[layerId] = 0.f * u.potential;
                } else {
                    auto const lowLayerId = layer.lowLayerId();
                    assert(lowLayerId >= 0);
                    assert(lowLayerId < layerId);
                    layerOffsets[layerId] = layerOffsets[lowLayerId] + findBalanceOffset(layer);
                }
            }
            auto const& layerOfNode = ctx_.lStructure.layerOfNode();
            for (std::size_t nodeId = 0; nodeId < ctx_.potentials.size(); ++nodeId) {
                ctx_.potentials[nodeId] += layerOffsets[layerOfNode[nodeId]];
            }
        }
    private:
        struct LayerStepPoint {
            Real<u.potential> offset;
            Real<u.force> force;
            Real<u.conductivity> conductivity;

            [[nodiscard]]
            Real<u.potential> nextOffset() const {
                return offset + force / conductivity;
            }
        };

        [[nodiscard]]
        LayerStepPoint pointAt(Layer const& layer, Real<u.potential> const offset) const {
            Real<u.force> force = layer.cumulatedWeight();
            Real<u.conductivity> conductivity = 0.f * u.conductivity;
            for (auto const& contact : layer.lowContactIds().subSpanOf(ctx_.lStructure.lowContacts())) {
                Real<u.potential> const localPotential = offset + ctx_.potentials[contact.localIndex()];
                Real<u.potential> const otherPotential = ctx_.potentials[contact.otherIndex()];
                auto const forceStats = contact.forceStats(localPotential, otherPotential);
                force += forceStats.force();
                conductivity += forceStats.conductivity;
            }
            return { offset, force, conductivity };
        }

        [[nodiscard]]
        Real<u.potential> findBalanceOffset(Layer const& layer) const {
            Real<u.force> const maxForceError = targetErrorFactor * ctx_.config().targetMaxError() * layer.cumulatedWeight();
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
