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

#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/force1/detail/ForceBalancer.hpp>
#include <gustave/solvers/force1/detail/SolutionData.hpp>
#include <gustave/solvers/force1/solutionUtils/Contacts.hpp>
#include <gustave/solvers/force1/solutionUtils/Nodes.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
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

        using ForceBalancer = detail::ForceBalancer<cfg>;
        using SolutionData = detail::SolutionData<cfg>;

        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;
    public:
        using Basis = SolutionBasis<cfg>;
        using Contacts = SolutionUtils::Contacts<cfg>;
        using Nodes = SolutionUtils::Nodes<cfg>;
        using Structure = typename Basis::Structure;

        using ContactIndex = typename Structure::ContactIndex;
        using Link = typename Structure::Link;

        [[nodiscard]]
        explicit Solution(std::shared_ptr<const Basis> basis)
            : data_{ std::move(basis) }
        {}

        [[nodiscard]]
        explicit Solution(std::shared_ptr<const Basis> basis, ForceBalancer balancer)
            : data_{ std::move(basis), std::move(balancer) }
        {}

        [[nodiscard]]
        Real<u.one> maxRelativeError() const {
            return data_.forceRepartition().maxRelativeError();
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ data_ };
        }

        [[nodiscard]]
        Nodes nodes() const {
            return Nodes{ data_ };
        }
    private:
        SolutionData data_;
    };
}
