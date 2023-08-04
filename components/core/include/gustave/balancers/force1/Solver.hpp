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

#include <gustave/balancers/force1/Solution.hpp>
#include <gustave/balancers/force1/SolutionBasis.hpp>
#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/model/ContactArea.hpp>
#include <gustave/model/Material.hpp>
#include <gustave/model/Node.hpp>
#include <gustave/model/Structure.hpp>

namespace Gustave::Balancers::Force1 {

    template<Cfg::cLibConfig auto cfg>
    class Solver {
    private:
        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;

        using ContactIndex = Cfg::ContactIndex<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;

        using Node = Model::Node<cfg>;
        using Structure = Model::Structure<cfg>;
        using NodeInfo = Solution<cfg>::NodeInfo;
        using ContactInfo = Solution<cfg>::ContactInfo;
    public:
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

        [[nodiscard]]
        Solver(Structure const& structure, Vector3<u.acceleration> const& g, Config const& config)
            : Solver(std::make_shared<SolutionBasis<cfg>>(structure, g), config)
        {
            
        }

        [[nodiscard]]
        Solver(std::shared_ptr<SolutionBasis<cfg>> basis, Config const& config)
            : structure_{ basis->structure() }
            , basis_{ basis.get() }
            , solution_{ std::make_unique<Solution<cfg>>(std::move(basis)) }
        {
            std::vector<Node> const& nodes = structure_.nodes();
            std::vector<Real<u.potential>> const& potentials = basis_->potentials();
            std::vector<Real<u.potential>> nextPotentials = potentials;
            constexpr Real<u.one> convergenceFactor = 0.5f;
            IterationIndex iteration = 0;
            do {
                Real<u.one> currentMaxError = 0.f;
                for (NodeIndex id = 0; id < nodes.size(); ++id) {
                    Node const& node = nodes[id];
                    if (!node.isFoundation) {
                        auto const nodeStats = solution_->statsOf(id);
                        nextPotentials[id] = potentials[id] - nodeStats.force / nodeStats.derivative * convergenceFactor;
                        currentMaxError = rt.max(currentMaxError, nodeStats.relativeError());
                    }
                }
                if (currentMaxError >= config.targetMaxError()) {
                    basis_->swapPotentials(nextPotentials);
                    ++iteration;
                } else {
                    break;
                }
            } while (iteration < config.maxIterations());
        }

        [[nodiscard]]
        Solution<cfg> const& solution() const {
            return *solution_;
        }
    private:
        Structure const& structure_;
        SolutionBasis<cfg>* basis_;
        std::unique_ptr<Solution<cfg> const> solution_;
    };
}