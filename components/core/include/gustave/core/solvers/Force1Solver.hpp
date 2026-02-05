/* This file is part of Gustave, a structural integrity library for video games.
 * 
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <utility>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/BasicStepRunner.hpp>
#include <gustave/core/solvers/force1Solver/detail/ClusterStepRunner.hpp>
#include <gustave/core/solvers/force1Solver/detail/LayerStepRunner.hpp>
#include <gustave/core/solvers/force1Solver/detail/SolverRunContext.hpp>
#include <gustave/core/solvers/force1Solver/Config.hpp>
#include <gustave/core/solvers/force1Solver/Solution.hpp>
#include <gustave/core/solvers/Structure.hpp>

namespace gustave::core::solvers {
    template<cfg::cLibConfig auto libCfg>
    class Force1Solver {
    private:
        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using NodeIndex = cfg::NodeIndex<libCfg>;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;

        using BasicStepRunner = force1Solver::detail::BasicStepRunner<libCfg>;
        using ClusterStepRunner = force1Solver::detail::ClusterStepRunner<libCfg>;
        using LayerStepRunner = force1Solver::detail::LayerStepRunner<libCfg>;
        using SolverRunContext = force1Solver::detail::SolverRunContext<libCfg>;

        using BasicStepResult = BasicStepRunner::StepResult;
        using F1Structure = SolverRunContext::F1Structure;
    public:
        using Config = force1Solver::Config<libCfg>;
        using Solution = force1Solver::Solution<libCfg>;
        using Structure = solvers::Structure<libCfg>;

        using Basis = Solution::Basis;
        using IterationIndex = SolverRunContext::IterationIndex;
        using Node = Structure::Node;

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

        [[nodiscard]]
        explicit Force1Solver(Config const& config)
            : config_{ std::make_shared<Config const>(config) }
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
            SolverRunContext ctx{ *structure, *config_ };
            if (!isSolvable(ctx)) {
                return makeInvalidResult(std::move(ctx));
            }
            BasicStepRunner basicRunner{ ctx };
            ClusterStepRunner clusterRunner{ ctx };
            LayerStepRunner layerRunner{ ctx };
            do {
                layerRunner.runStep();
                for (auto const& cStructure : ctx.cStructures) {
                    clusterRunner.runStep(cStructure);
                }
                BasicStepResult const stepResult = basicRunner.runStep();
                if (stepResult.isBelowTargetError) {
                    return makeValidResult(std::move(ctx), std::move(structure));
                }
            } while (ctx.iterationIndex < config_->maxIterations());
            return makeInvalidResult(std::move(ctx));
        }
    private:
        [[nodiscard]]
        static bool isSolvable(SolverRunContext const& ctx) {
            return ctx.lStructure.reachedCount() == ctx.fStructure.fNodes().size();
        }

        [[nodiscard]]
        Result makeInvalidResult(SolverRunContext&& ctx) const {
            return Result{ ctx.iterationIndex, nullptr };
        }

        [[nodiscard]]
        Result makeValidResult(SolverRunContext&& ctx, std::shared_ptr<Structure const>&& structure) const {
            auto basis = std::make_shared<Basis const>(std::move(structure), config_, std::move(ctx.potentials));
            return Result{ ctx.iterationIndex, std::make_shared<Solution const>(std::move(basis), std::move(ctx.fStructure)) };
        }

        std::shared_ptr<Config const> config_;
    };
}
