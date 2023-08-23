/* This file is part of Gustave, a structural integrity library for video games.
 * 
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <gustave/solvers/force1/detail/ForceBalancer.hpp>
#include <gustave/solvers/force1/detail/ForceRepartition.hpp>
#include <gustave/solvers/force1/Solution.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/SolverNode.hpp>
#include <gustave/solvers/SolverStructure.hpp>

namespace Gustave::Solvers::Force1 {
    template<Cfg::cLibConfig auto cfg>
    class Solver {
    private:
        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = typename Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = typename Cfg::Vector3<cfg, unit>;

        using ContactIndex = typename Cfg::ContactIndex<cfg>;
        using NodeIndex = typename Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = typename Cfg::NormalizedVector3<cfg>;
    public:
        using ForceBalancer = detail::ForceBalancer<cfg>;
        using ForceRepartition = detail::ForceRepartition<cfg>;
        using IterationIndex = std::uint64_t;

        class Config {
        public:
            [[nodiscard]]
            Config(std::uint64_t maxIterations, Real<u.one> targetMaxError)
                : maxIterations_{maxIterations}
                , targetMaxError_{targetMaxError}
            {
                assert(targetMaxError > 0.f);
            }

            [[nodiscard]]
            IterationIndex maxIterations() const {
                return maxIterations_;
            }

            [[nodiscard]]
            Real<u.one> targetMaxError() const {
                return targetMaxError_;
            }
        private:
            IterationIndex maxIterations_;
            Real<u.one> targetMaxError_;
        };

        class Result {
        public:
            [[nodiscard]]
            Result(IterationIndex iterations, std::shared_ptr<Solution<cfg> const> solution)
                : iterations_{ iterations }
                , solution_{ solution }
            {}

            [[nodiscard]]
            IterationIndex iterations() const {
                return iterations_;
            }

            [[nodiscard]]
            Solution<cfg> const& solution() const {
                return *solution_;
            }

            [[nodiscard]]
            std::shared_ptr<Solution<cfg> const> const& solutionPtr() const {
                return solution_;
            }
        private:
            IterationIndex iterations_;
            std::shared_ptr<Solution<cfg> const> solution_;
        };

        [[nodiscard]]
        static Result run(SolverProblem<cfg> problem, std::shared_ptr<Config const> config) {
            return Solver{ std::move(problem), std::move(config) }.result();
        }

        [[nodiscard]]
        Result result() && {
            return { iteration_, std::make_shared<Solution<cfg> const>(std::move(balancer_), std::move(potentials_)) };
        }
    private:
        std::shared_ptr<Config const> config_;
        SolverProblem<cfg> problem_;
        ForceBalancer balancer_;
        IterationIndex iteration_;
        std::vector<Real<u.potential>> potentials_;
        std::vector<Real<u.potential>> nextPotentials_;

        [[nodiscard]]
        Solver(SolverProblem<cfg> problem, std::shared_ptr<Config const> config)
            : config_{ std::move(config) }
            , problem_{ std::move(problem) }
            , balancer_{ problem_ }
            , iteration_{ 0 }
            , potentials_(problem_.structure().nodes().size(), 0.f * u.potential)
            , nextPotentials_{ potentials_ }
        {
            std::vector<SolverNode<cfg>> const& nodes = problem_.structure().nodes();
            constexpr Real<u.one> convergenceFactor = 0.5f;
            do {
                ForceRepartition repartition{ balancer_, potentials_ };
                Real<u.one> currentMaxError = 0.f;
                for (NodeIndex id = 0; id < nodes.size(); ++id) {
                    SolverNode<cfg> const& node = nodes[id];
                    if (!node.isFoundation) {
                        auto const nodeStats = repartition.statsOf(id);
                        nextPotentials_[id] = potentials_[id] - nodeStats.force() / nodeStats.derivative() * convergenceFactor;
                        currentMaxError = rt.max(currentMaxError, nodeStats.relativeError());
                    }
                }
                if (currentMaxError >= config_->targetMaxError()) {
                    potentials_.swap(nextPotentials_);
                    ++iteration_;
                } else {
                    break;
                }
            } while (iteration_ < config_->maxIterations());
        }
    };
}
