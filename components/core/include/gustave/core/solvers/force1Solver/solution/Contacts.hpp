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

#include <sstream>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodeInfo.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodeStats.hpp>
#include <gustave/core/solvers/force1Solver/detail/SolutionData.hpp>
#include <gustave/core/solvers/force1Solver/solution/ContactReference.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>

namespace gustave::core::solvers::force1Solver::solution {
    template<cfg::cLibConfig auto libCfg>
    class Contacts {
    private:
        using SolutionData = detail::SolutionData<libCfg>;
        using Structure = solvers::Structure<libCfg>;

        using LinkIndex = typename Structure::LinkIndex;
    public:
        using ContactIndex = typename Structure::ContactIndex;
        using ContactReference = solution::ContactReference<libCfg>;

        [[nodiscard]]
        explicit Contacts(SolutionData const& solution)
            : solution_{ &solution }
        {}

        [[nodiscard]]
        ContactReference at(ContactIndex const& index) const {
            LinkIndex linkId = index.linkIndex;
            std::size_t linksCount = solution_->basis().structure().links().size();
            if (linkId >= linksCount) {
                std::stringstream msg;
                msg << "Index " << linkId << " is out of range (size: " << linksCount << ").";
                throw std::out_of_range(msg.str());
            }
            return ContactReference{ *solution_, index };
        }
    private:
        SolutionData const* solution_;
    };
}
