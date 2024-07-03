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
#include <gustave/core/solvers/force1Solver/detail/BasicNodeEvaluator.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodeBalancer.hpp>
#include <gustave/core/solvers/force1Solver/detail/SolverRunContext.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class BasicStepRunner {
    private:
        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using NodeIndex = cfg::NodeIndex<libCfg>;

        using SolverRunContext = detail::SolverRunContext<libCfg>;

        using NodeBalancer = detail::NodeBalancer<libCfg>;
        using NodeEvaluator = detail::BasicNodeEvaluator<libCfg>;
    public:
        static constexpr Real<u.one> targetErrorFactor = 0.75f;

        struct StepResult {
            bool isBelowTargetError;
        };

        [[nodiscard]]
        explicit BasicStepRunner(SolverRunContext& ctx)
            : ctx_{ ctx }
        {}

        StepResult runStep() {
            Real<u.one> currentMaxError = 0.f;
            auto const& fNodes = ctx_.fStructure.fNodes();
            auto const balancer = NodeBalancer{ targetErrorFactor * ctx_.config().targetMaxError() };
            for (NodeIndex id = 0; id < fNodes.size(); ++id) {
                auto const& fNode = fNodes[id];
                if (!fNode.isFoundation) {
                    auto const evaluator = NodeEvaluator{ ctx_.potentials, ctx_.fStructure.fContactsOf(id), fNode.weight };
                    auto const balanceResult = balancer.findBalanceOffset(evaluator, ctx_.potentials[id]);
                    ctx_.nextPotentials[id] = balanceResult.offset;
                    currentMaxError = rt.max(currentMaxError, balanceResult.initialForce / fNode.weight);
                }
            }
            if (currentMaxError >= ctx_.config().targetMaxError()) {
                ctx_.potentials.swap(ctx_.nextPotentials);
                ++ctx_.iterationIndex;
                return StepResult{ false };
            } else {
                return StepResult{ true };
            }
        }
    private:
        SolverRunContext& ctx_;
    };
}
