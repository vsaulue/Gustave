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

#include <cassert>
#include <memory>
#include <span>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/SolverProblem.hpp>
#include <gustave/solvers/SolverStructure.hpp>

namespace Gustave::Solvers::Force1 {
    template<Cfg::cLibConfig auto cfg>
    class SolutionBasis {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        [[nodiscard]]
        SolutionBasis(std::shared_ptr<SolverStructure<cfg> const> structure, Vector3<u.acceleration> const& g)
            : problem_{ g, std::move(structure) }
            , potentials_{ problem_.structure().nodes().size(), 0.f * u.potential }
        {}

        [[nodiscard]]
        SolverStructure<cfg> const& structure() const {
            return problem_.structure();
        }

        [[nodiscard]]
        std::vector<Real<u.potential>> const& potentials() const {
            return potentials_;
        }

        [[nodiscard]]
        std::span<Real<u.potential>> spanPotentials() {
            return potentials_;
        }

        void swapPotentials(std::vector<Real<u.potential>>& newPotentials) {
            potentials_.swap(newPotentials);
            checkPotentials();
        }

        [[nodiscard]]
        Vector3<u.acceleration> const& g() const {
            return problem_.g();
        }
    private:
        SolverProblem<cfg> problem_;
        std::vector<Real<u.potential>> potentials_;

        void checkPotentials() const {
            assert(potentials_.size() == problem_.structure().nodes().size());
        }
    };

}
