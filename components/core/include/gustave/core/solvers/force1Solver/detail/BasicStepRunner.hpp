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
#include <gustave/core/solvers/force1Solver/detail/ForceRepartition.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class BasicStepRunner {
    private:
        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using NodeIndex = cfg::NodeIndex<libCfg>;

        using ForceRepartition = detail::ForceRepartition<libCfg>;
        using SolverRunContext = detail::SolverRunContext<libCfg>;

        using NodeStats = typename ForceRepartition::NodeStats;
        using Node = typename ForceRepartition::Structure::Node;

        struct NodeStepResult {
            Real<u.one> currentNodeError;
            Real<u.potential> nextPotential;
        };

        struct NodeStepPoint {
            Real<u.potential> potential;
            NodeStats stats;

            [[nodiscard]]
            Real<u.force> force() const {
                return stats.force();
            }

            [[nodiscard]]
            Real<u.potential> nextPotential() const {
                return potential - stats.force() / stats.derivative();
            }
        };
    public:
        static constexpr Real<u.one> targetErrorFactor = 0.75f;

        struct StepResult {
            Real<u.one> currentMaxError;
        };

        [[nodiscard]]
        explicit BasicStepRunner(SolverRunContext& ctx)
            : ctx_{ ctx }
        {}

        StepResult runStep() {
            Real<u.one> const stepTargetError = targetErrorFactor * ctx_.config().targetMaxError();
            Real<u.one> currentMaxError = 0.f;
            auto const nodes = ctx_.fStructure.structure().nodes();
            for (NodeIndex id = 0; id < nodes.size(); ++id) {
                Node const& node = nodes[id];
                if (!node.isFoundation) {
                    auto const& fNode = ctx_.fStructure.fNodes()[id];
                    Real<u.force> const nodeForceError = stepTargetError * fNode.weight;
                    auto const nodeStepResult = runNodeStep(id, nodeForceError);
                    ctx_.nextPotentials[id] = nodeStepResult.nextPotential;
                    currentMaxError = rt.max(currentMaxError, nodeStepResult.currentNodeError);
                }
            }
            return StepResult{ currentMaxError };
        }
    private:
        NodeStepResult runNodeStep(NodeIndex const nodeId, Real<u.force> maxForceError) {
            ForceRepartition const repartition{ ctx_.fStructure, ctx_.potentials };
            auto pointAt = [&](Real<u.potential> potential) -> NodeStepPoint {
                return { potential, repartition.statsOf(nodeId, potential) };
            };
            NodeStepPoint curPoint = pointAt(ctx_.potentials[nodeId]);
            Real<u.one> const curMaxError = curPoint.stats.relativeError();
            NodeStepPoint nextPoint = pointAt(curPoint.nextPotential());
            if (rt.abs(nextPoint.force()) <= maxForceError) {
                return { curMaxError, nextPoint.potential };
            }
            bool const startSignBit = rt.signBit(curPoint.force());
            while (rt.signBit(nextPoint.force()) == startSignBit) {
                curPoint = nextPoint;
                nextPoint = pointAt(curPoint.nextPotential());
                assert(curPoint.potential != nextPoint.potential); // float resolution too low ?
                if (rt.abs(nextPoint.force()) <= maxForceError) {
                    return { curMaxError, nextPoint.potential };
                }
            }
            do {
                Real<u.resistance> const invDerivative = (nextPoint.potential - curPoint.potential) / (nextPoint.force() - curPoint.force());
                NodeStepPoint const midPoint = pointAt(curPoint.potential - invDerivative * curPoint.force());
                assert(midPoint.potential != curPoint.potential);  // float resolution too low ?
                assert(midPoint.potential != nextPoint.potential); // float resolution too low ?
                if (rt.abs(midPoint.force()) <= maxForceError) {
                    return { curMaxError, midPoint.potential };
                }
                if (rt.signBit(midPoint.force()) == startSignBit) {
                    curPoint = midPoint;
                } else {
                    nextPoint = midPoint;
                }
            } while (true);
        }

        SolverRunContext& ctx_;
    };
}
