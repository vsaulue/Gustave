/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/ForceRepartition.hpp>
#include <gustave/core/solvers/force1Solver/SolutionBasis.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class SolutionData {
    public:
        using Basis = SolutionBasis<libCfg>;
        using F1Structure = detail::F1Structure<libCfg>;
        using ForceRepartition = detail::ForceRepartition<libCfg>;

        [[nodiscard]]
        explicit SolutionData(std::shared_ptr<const Basis>&& basis)
            : basis_{ std::move(basis) }
            , fStructure_{ basis_->structure(), basis_->config() }
        {}

        [[nodiscard]]
        explicit SolutionData(std::shared_ptr<const Basis>&& basis, F1Structure&& balancer)
            : basis_{ std::move(basis) }
            , fStructure_{ std::move(balancer) }
        {
            assert(&basis_->structure() == &fStructure_.structure());
            assert(&basis_->config() == &fStructure_.config());
        }

        [[nodiscard]]
        Basis const& basis() const {
            return *basis_;
        }

        [[nodiscard]]
        F1Structure const& fStructure() const {
            return fStructure_;
        }

        [[nodiscard]]
        ForceRepartition forceRepartition() const {
            return ForceRepartition{ fStructure_, basis_->potentials() };
        }
    private:
        std::shared_ptr<Basis const> basis_;
        F1Structure fStructure_;
    };
}
