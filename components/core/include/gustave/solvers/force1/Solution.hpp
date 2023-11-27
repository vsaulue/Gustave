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
#include <gustave/solvers/force1/detail/ForceBalancer.hpp>
#include <gustave/solvers/force1/detail/ForceRepartition.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/SolverStructure.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Solvers::Force1 {
    template<Cfg::cLibConfig auto cfg>
    class Solution {
    private:
        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;

        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;

        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;
    public:
        using Basis = SolutionBasis<cfg>;
        using ForceBalancer = detail::ForceBalancer<cfg>;
        using ForceRepartition = detail::ForceRepartition<cfg>;
        using NodeInfo = typename ForceBalancer::NodeInfo;
        using NodeStats = typename ForceRepartition::NodeStats;

        [[nodiscard]]
        explicit Solution(std::shared_ptr<const Basis> basis)
            : basis_{ std::move(basis) }
            , forceBalancer_{ basis_->structure(), basis_->config() }
        {}

        [[nodiscard]]
        explicit Solution(std::shared_ptr<const Basis> basis, ForceBalancer balancer)
            : basis_{ std::move(basis) }
            , forceBalancer_{ std::move(balancer) }
        {
            assert(&basis_->structure() == &forceBalancer_.structure());
            assert(&basis_->config() == &forceBalancer_.config());
        }

        [[nodiscard]]
        std::shared_ptr<const Basis> const& basis() const {
            return basis_;
        }

        [[nodiscard]]
        SolverStructure<cfg> const& structure() const {
            return basis_->structure();
        }

        [[nodiscard]]
        std::vector<Real<u.potential>> const& potentials() const {
            return basis_->potentials();
        }

        [[nodiscard]]
        Vector3<u.acceleration> const& g() const {
            return basis_->g();
        }

        [[nodiscard]]
        Vector3<u.force> forceVector(NodeIndex to, NodeIndex from) const {
            return forceRepartition().forceVector(to, from);
        }

        [[nodiscard]]
        Real<u.one> maxRelativeError() const {
            return forceRepartition().maxRelativeError();
        }

        [[nodiscard]]
        Real<u.one> relativeErrorOf(NodeIndex id) const {
            return forceRepartition().relativeErrorOf(id);
        }

        [[nodiscard]]
        NodeStats statsOf(NodeIndex id) const {
            return forceRepartition().statsOf(id);
        }

        [[nodiscard]]
        Real<u.one> sumRelativeError() const {
            return forceRepartition().sumRelativeError();
        }
    private:
        std::shared_ptr<Basis const> basis_;
        ForceBalancer forceBalancer_;

        [[nodiscard]]
        auto forceRepartition() const -> ForceRepartition {
            return { forceBalancer_, basis_->potentials() };
        }
    };
}
