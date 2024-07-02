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

#include <cassert>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1BasicContact.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class LocalContact {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using F1BasicContact = detail::F1BasicContact<libCfg>;

        using ForceStats = typename F1BasicContact::ForceStats;
        using NodeIndex = typename F1BasicContact::NodeIndex;

        [[nodiscard]]
        explicit LocalContact(F1BasicContact const& fContact, NodeIndex localIndex)
            : fContact_{ fContact }
            , localIndex_{ localIndex }
        {
            assert(localIndex != fContact.otherIndex());
        }

        [[nodiscard]]
        ForceStats forceStats(Real<u.potential> sourcePotential, Real<u.potential> otherPotential) const {
            return fContact_.forceStats(sourcePotential, otherPotential);
        }

        [[nodiscard]]
        NodeIndex localIndex() const {
            return localIndex_;
        }

        [[nodiscard]]
        NodeIndex otherIndex() const {
            return fContact_.otherIndex();
        }

        [[nodiscard]]
        bool operator==(LocalContact const&) const = default;
    private:
        F1BasicContact fContact_;
        NodeIndex localIndex_;
    };
}
