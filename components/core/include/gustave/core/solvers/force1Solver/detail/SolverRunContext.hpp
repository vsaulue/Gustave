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

#include <cstdint>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/ClusterStructure.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/LayerStructure.hpp>
#include <gustave/core/solvers/force1Solver/Config.hpp>
#include <gustave/core/solvers/Structure.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    struct SolverRunContext {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using NodeIndex = cfg::NodeIndex<libCfg>;
    public:
        using ClusterStructure = detail::ClusterStructure<libCfg>;
        using Config = force1Solver::Config<libCfg>;
        using F1Structure = detail::F1Structure<libCfg>;
        using IterationIndex = std::uint64_t;
        using LayerStructure = detail::LayerStructure<libCfg>;
        using Structure = solvers::Structure<libCfg>;

        [[nodiscard]]
        explicit SolverRunContext(Structure const& structure, Config const& config)
            : fStructure{ structure, config }
            , lStructure{ fStructure }
            , cStructures{ initClusterStuctures(fStructure) }
            , iterationIndex{ 0 }
            , potentials(structure.nodes().size(), 0.f * u.potential)
            , nextPotentials(structure.nodes().size(), 0.f * u.potential)
        {}

        [[nodiscard]]
        Config const& config() const {
            return fStructure.config();
        }

        F1Structure fStructure;
        LayerStructure lStructure;
        std::vector<ClusterStructure> cStructures;
        IterationIndex iterationIndex;
        std::vector<Real<u.potential>> potentials;
        std::vector<Real<u.potential>> nextPotentials;
    private:
        [[nodiscard]]
        static std::vector<ClusterStructure> initClusterStuctures(F1Structure const& fStructure) {
            static constexpr auto maxWidth = std::numeric_limits<NodeIndex>::max() / 2;
            std::vector<ClusterStructure> result;
            for (NodeIndex width = 3; width < maxWidth; width = 1 + 2 * width) {
                result.emplace_back(fStructure, width);
                if (result.back().clusters().size() < 8) {
                    result.pop_back();
                    break;
                }
            }
            return result;
        }
    };
}
