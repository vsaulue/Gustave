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
#include <gustave/solvers/force1/Config.hpp>
#include <gustave/solvers/force1/Solution.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/Structure.hpp>

namespace Gustave::Solvers {
    template<Cfg::cLibConfig auto cfg>
    class Force1Solver {
    private:
        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = typename Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = typename Cfg::Vector3<cfg, unit>;

        using NodeIndex = typename Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = typename Cfg::NormalizedVector3<cfg>;
    public:
        using Structure = Solvers::Structure<cfg>;

        using Basis = Force1::SolutionBasis<cfg>;
        using Config = Force1::Config<cfg>;
        using ForceBalancer = Force1::detail::ForceBalancer<cfg>;
        using ForceRepartition = Force1::detail::ForceRepartition<cfg>;
        using IterationIndex = std::uint64_t;
        using Node = typename Structure::Node;
        using Solution = Force1::Solution<cfg>;

        class Result {
        public:
            [[nodiscard]]
            Result(IterationIndex iterations, std::shared_ptr<Solution const> solution)
                : iterations_{ iterations }
                , solution_{ solution }
            {}

            [[nodiscard]]
            IterationIndex iterations() const {
                return iterations_;
            }

            [[nodiscard]]
            Solution const& solution() const {
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
        explicit Force1Solver(std::shared_ptr<Config const> config)
            : config_{ std::move(config) }
        {
            assert(config_);
        }

        [[nodiscard]]
        Result run(std::shared_ptr<Structure const> structure) const {
            ForceBalancer balancer{ *structure, *config_ };
            IterationIndex iteration = 0;
            std::vector<Real<u.potential>> potentials(balancer.structure().nodes().size(), 0.f * u.potential);
            std::vector<Real<u.potential>> nextPotentials{ potentials };

            std::vector<Node> const& nodes = balancer.structure().nodes();
            constexpr Real<u.one> convergenceFactor = 0.5f;
            do {
                ForceRepartition repartition{ balancer, potentials };
                Real<u.one> currentMaxError = 0.f;
                for (NodeIndex id = 0; id < nodes.size(); ++id) {
                    Node const& node = nodes[id];
                    if (!node.isFoundation) {
                        auto const nodeStats = repartition.statsOf(id);
                        nextPotentials[id] = potentials[id] - nodeStats.force() / nodeStats.derivative() * convergenceFactor;
                        currentMaxError = rt.max(currentMaxError, nodeStats.relativeError());
                    }
                }
                if (currentMaxError >= balancer.config().targetMaxError()) {
                    potentials.swap(nextPotentials);
                    ++iteration;
                }
                else {
                    break;
                }
            } while (iteration < balancer.config().maxIterations());

            auto basis = std::make_shared<Basis const>(std::move(structure), config_, std::move(potentials));
            return Result{ iteration, std::make_shared<Solution const>(std::move(basis), std::move(balancer)) };
        }
    private:
        std::shared_ptr<Config const> config_;
    };
}
