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

#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/SolutionData.hpp>
#include <gustave/core/solvers/force1Solver/solution/Contacts.hpp>
#include <gustave/core/solvers/force1Solver/solution/Nodes.hpp>
#include <gustave/core/solvers/force1Solver/SolutionBasis.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::solvers::force1Solver {
    template<cfg::cLibConfig auto libCfg>
    class Solution {
    private:
        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;

        using F1Structure = detail::F1Structure<libCfg>;
        using SolutionData = detail::SolutionData<libCfg>;

        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;
    public:
        using Basis = SolutionBasis<libCfg>;
        using Contacts = solution::Contacts<libCfg>;
        using Nodes = solution::Nodes<libCfg>;

        using ContactIndex = Basis::Structure::ContactIndex;
        using ContactReference = Contacts::ContactReference;
        using Link = Basis::Structure::Link;
        using NodeIndex = Nodes::NodeReference::NodeIndex;
        using NodeReference = Nodes::NodeReference;
        using Structure = Basis::Structure;

        [[nodiscard]]
        explicit Solution(std::shared_ptr<const Basis> basis)
            : data_{ std::move(basis) }
        {}

        [[nodiscard]]
        explicit Solution(std::shared_ptr<const Basis> basis, F1Structure fStructure)
            : data_{ std::move(basis), std::move(fStructure) }
        {}

        [[nodiscard]]
        Basis const& basis() const {
            return data_.basis();
        }

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
