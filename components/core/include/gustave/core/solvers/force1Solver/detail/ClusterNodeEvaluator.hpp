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

#include <span>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/LocalContact.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodePoint.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg_>
    class ClusterNodeEvaluator {
    private:
        template<cfg::cUnitOf<libCfg_> auto unit>
        using Real = cfg::Real<libCfg_, unit>;

        static constexpr auto u = cfg::units(libCfg_);
    public:
        using LocalContact = detail::LocalContact<libCfg_>;
        using NodePoint = detail::NodePoint<libCfg_>;

        using Potentials = std::span<Real<u.potential> const>;
        using Contacts = std::span<LocalContact const>;

        [[nodiscard]]
        explicit ClusterNodeEvaluator(Potentials const& potentials, Contacts const& contacts, Real<u.force> weight)
            : potentials_{ potentials }
            , contacts_{ contacts }
            , weight_{ weight }
        {}

        [[nodiscard]]
        NodePoint pointAt(Real<u.potential> const offset) const {
            Real<u.force> force = weight_;
            Real<u.conductivity> conductivity = 0.f * u.conductivity;
            for (auto const& contact : contacts_) {
                Real<u.potential> const localPotential = offset + potentials_[contact.localIndex()];
                Real<u.potential> const otherPotential = potentials_[contact.otherIndex()];
                auto const forceStats = contact.forceStats(localPotential, otherPotential);
                force += forceStats.force();
                conductivity += forceStats.conductivity;
            }
            return NodePoint{ offset, force, conductivity };
        }

        [[nodiscard]]
        Real<u.force> weight() const {
            return weight_;
        }
    private:
        Potentials potentials_;
        Contacts contacts_;
        Real<u.force> weight_;
    };
}
