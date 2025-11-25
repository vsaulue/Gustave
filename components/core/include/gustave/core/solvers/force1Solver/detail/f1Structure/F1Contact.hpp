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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1BasicContact.hpp>

namespace gustave::core::solvers::force1Solver::detail::f1Structure {
    template<cfg::cLibConfig auto libCfg>
    class F1Contact {
    private:
        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
    public:
        using F1BasicContact = detail::F1BasicContact<libCfg>;

        using ForceStats = F1BasicContact::ForceStats;
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        [[nodiscard]]
        explicit F1Contact(NodeIndex otherIndex, LinkIndex linkIndex, Real<u.conductivity> cPlus, Real<u.conductivity> cMinus)
            : basicContact_{ otherIndex, cPlus, cMinus }
            , linkIndex_{ linkIndex }
        {}

        [[nodiscard]]
        F1BasicContact const& basicContact() const {
            return basicContact_;
        }

        [[nodiscard]]
        Real<u.conductivity> cMinus() const {
            return basicContact_.cMinus();
        }

        [[nodiscard]]
        Real<u.conductivity> cPlus() const {
            return basicContact_.cPlus();
        }

        [[nodiscard]]
        ForceStats forceStats(Real<u.potential> sourcePotential, Real<u.potential> otherPotential) const {
            return basicContact_.forceStats(sourcePotential, otherPotential);
        }

        [[nodiscard]]
        LinkIndex linkIndex() const {
            return linkIndex_;
        }

        [[nodiscard]]
        NodeIndex otherIndex() const {
            return basicContact_.otherIndex();
        }

        [[nodiscard]]
        bool operator==(F1Contact const&) const = default;
    private:
        F1BasicContact basicContact_;
        LinkIndex linkIndex_;
    };
}
