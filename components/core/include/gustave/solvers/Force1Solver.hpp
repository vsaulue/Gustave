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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stack>
#include <utility>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/force1Solver/detail/ForceBalancer.hpp>
#include <gustave/solvers/force1Solver/detail/ForceRepartition.hpp>
#include <gustave/solvers/force1Solver/Config.hpp>
#include <gustave/solvers/force1Solver/Solution.hpp>
#include <gustave/solvers/force1Solver/SolutionBasis.hpp>
#include <gustave/solvers/Structure.hpp>

namespace gustave::solvers {
    template<cfg::cLibConfig auto libCfg>
    class Force1Solver {
    private:
        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = typename cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = typename cfg::Vector3<libCfg, unit>;

        using NodeIndex = typename cfg::NodeIndex<libCfg>;
        using NormalizedVector3 = typename cfg::NormalizedVector3<libCfg>;
    public:
        using Structure = solvers::Structure<libCfg>;

        using Basis = force1Solver::SolutionBasis<libCfg>;
        using Config = force1Solver::Config<libCfg>;
        using ForceBalancer = force1Solver::detail::ForceBalancer<libCfg>;
        using ForceRepartition = force1Solver::detail::ForceRepartition<libCfg>;
        using IterationIndex = std::uint64_t;
        using Node = typename Structure::Node;
        using Solution = force1Solver::Solution<libCfg>;

        class Result {
        public:
            [[nodiscard]]
            Result(IterationIndex iterations, std::shared_ptr<Solution const> solution)
                : iterations_{ iterations }
                , solution_{ solution }
            {}

            [[nodiscard]]
            bool isSolved() const {
                return solution_ != nullptr;
            }

            [[nodiscard]]
            IterationIndex iterations() const {
                return iterations_;
            }

            [[nodiscard]]
            Solution const& solution() const {
                if (!isSolved()) {
                    throw std::logic_error("The solver didn't generate a valid solution.");
                }
                return *solution_;
            }

            [[nodiscard]]
            std::shared_ptr<Solution const> const& solutionPtr() const {
                return solution_;
            }
        private:
            IterationIndex iterations_;
            std::shared_ptr<Solution const> solution_;
        };

    private:
        struct SolvingContext {
            [[nodiscard]]
            explicit SolvingContext(Structure const& structure, Config const& config)
                : balancer{ structure, config }
                , iterationIndex{ 0 }
                , potentials(structure.nodes().size(), 0.f * u.potential)
                , nextPotentials(structure.nodes().size(), 0.f * u.potential)
            {}

            [[nodiscard]]
            std::vector<Node> const& nodes() const {
                return balancer.structure().nodes();
            }

            ForceBalancer balancer;
            IterationIndex iterationIndex;
            std::vector<Real<u.potential>> potentials;
            std::vector<Real<u.potential>> nextPotentials;
        };

        struct StepResult {
            Real<u.one> currentMaxError;
        };
    public:

        [[nodiscard]]
        explicit Force1Solver(std::shared_ptr<Config const> config)
            : config_{ std::move(config) }
        {
            assert(config_);
        }

        [[nodiscard]]
        Config const& config() const {
            return *config_;
        }

        [[nodiscard]]
        Result run(std::shared_ptr<Structure const> structure) const {
            if (structure == nullptr) {
                throw std::logic_error("Unexpected null pointer for argument 'structure'.");
            }
            SolvingContext ctx{ *structure, *config_ };
            if (!isSolvable(ctx.balancer)) {
                return makeInvalidResult(std::move(ctx));
            }

            do {
                StepResult const stepResult = runStep(ctx);
                if (stepResult.currentMaxError >= config_->targetMaxError()) {
                    ctx.potentials.swap(ctx.nextPotentials);
                    ++ctx.iterationIndex;
                } else {
                    return makeValidResult(std::move(ctx), std::move(structure));
                }
            } while (ctx.iterationIndex < config_->maxIterations());
            return makeInvalidResult(std::move(ctx));
        }
    private:
        [[nodiscard]]
        static bool isSolvable(ForceBalancer const& balancer) {
            std::vector<bool> reached(balancer.nodeInfos().size(), false);
            std::stack<NodeIndex> remainingIndices;
            std::size_t reachedCount = 0;
            for (std::size_t nodeId = 0; nodeId < reached.size(); ++nodeId) {
                Node const& node = balancer.structure().nodes()[nodeId];
                if (node.isFoundation) {
                    ++reachedCount;
                    reached[nodeId] = true;
                    remainingIndices.push(nodeId);
                }
            }
            while (!remainingIndices.empty()) {
                NodeIndex const nodeId = remainingIndices.top();
                remainingIndices.pop();
                for (auto const& contactInfo : balancer.nodeInfos()[nodeId].contacts) {
                    NodeIndex const otherNodeId = contactInfo.otherIndex();
                    if (!reached[otherNodeId]) {
                        reached[otherNodeId] = true;
                        ++reachedCount;
                        remainingIndices.push(otherNodeId);
                    }
                }
            }
            return reachedCount == balancer.nodeInfos().size();
        }

        [[nodiscard]]
        Result makeInvalidResult(SolvingContext&& ctx) const {
            return Result{ ctx.iterationIndex, nullptr };
        }

        [[nodiscard]]
        Result makeValidResult(SolvingContext&& ctx, std::shared_ptr<Structure const>&& structure) const {
            auto basis = std::make_shared<Basis const>(std::move(structure), config_, std::move(ctx.potentials));
            return Result{ ctx.iterationIndex, std::make_shared<Solution const>(std::move(basis), std::move(ctx.balancer)) };
        }

        [[nodiscard]]
        StepResult runStep(SolvingContext& ctx) const {
            static constexpr Real<u.one> convergenceFactor = 0.5f;
            ForceRepartition repartition{ ctx.balancer, ctx.potentials };
            Real<u.one> currentMaxError = 0.f;
            for (NodeIndex id = 0; id < ctx.nodes().size(); ++id) {
                Node const& node = ctx.nodes()[id];
                if (!node.isFoundation) {
                    auto const nodeStats = repartition.statsOf(id);
                    ctx.nextPotentials[id] = ctx.potentials[id] - nodeStats.force() / nodeStats.derivative() * convergenceFactor;
                    currentMaxError = rt.max(currentMaxError, nodeStats.relativeError());
                }
            }
            return StepResult{ currentMaxError };
        }

        std::shared_ptr<Config const> config_;
    };
}
