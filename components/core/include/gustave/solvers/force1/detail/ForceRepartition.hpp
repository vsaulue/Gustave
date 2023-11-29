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
#include <span>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/force1/detail/ForceBalancer.hpp>
#include <gustave/solvers/force1/detail/NodeStats.hpp>

namespace Gustave::Solvers::Force1::detail {
    template<Cfg::cLibConfig auto cfg>
    class ForceRepartition {
    private:
        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;

        using NodeIndex = Cfg::NodeIndex<cfg>;

        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;
    public:
        using Structure = Solvers::Structure<cfg>;

        using ContactInfo = detail::ContactInfo<cfg>;
        using ForceBalancer = detail::ForceBalancer<cfg>;
        using Node = typename Structure::Node;
        using NodeInfo = detail::NodeInfo<cfg>;
        using NodeStats = detail::NodeStats<cfg>;

        [[nodiscard]]
        ForceRepartition(ForceBalancer const& balancer, std::span<Real<u.potential> const> potentials)
            : balancer_{ balancer }
            , potentials_{ potentials }
        {
            assert(potentials_.size() == balancer.structure().nodes().size());
        }

        [[nodiscard]]
        Real<u.one> relativeErrorOf(NodeIndex id) const {
            return rt.abs(statsOf(id).relativeError());
        }

        [[nodiscard]]
        Real<u.one> maxRelativeError() const {
            Real<u.one> result = 0.f;
            for (NodeIndex id = 0; id < nodes().size(); ++id) {
                if (!nodes()[id].isFoundation) {
                    result = rt.max(result, relativeErrorOf(id));
                }
            }
            return result;
        }

        [[nodiscard]]
        Real<u.one> sumRelativeError() const {
            Real<u.one> result = 0.f;
            for (NodeIndex id = 0; id < nodes().size(); ++id) {
                if (!nodes()[id].isFoundation) {
                    result += relativeErrorOf(id);
                }
            }
            return result;
        }

        [[nodiscard]]
        NodeStats statsOf(NodeIndex id) const {
            NodeInfo const& info = nodeInfos()[id];
            Real<u.force> force = info.weight;
            Real<u.conductivity> derivative = 0.f * u.conductivity;

            Real<u.potential> const potential = potentials_[id];
            for (ContactInfo const& contactInfo : info.contacts) {
                ContactStats const contactStats = contactStatsOf(contactInfo, potential);
                derivative += contactStats.derivative();
                force += contactStats.force();
            }
            return { info, force, derivative };
        }

        [[nodiscard]]
        Real<u.force> forceCoord(NodeIndex to, NodeIndex from) const {
            Real<u.force> result = Real<u.force>::zero();
            NodeInfo const& toInfo = nodeInfos()[to];
            Real<u.potential> const toPotential = potentials_[to];
            for (ContactInfo const& contactInfo : toInfo.contacts) {
                if (contactInfo.otherIndex() == from) {
                    result += contactStatsOf(contactInfo, toPotential).force();
                }
            }
            return result;
        }

        [[nodiscard]]
        Vector3<u.force> forceVector(NodeIndex to, NodeIndex from) const {
            return forceCoord(to, from) * balancer_.normalizedG();
        }
    private:
        ForceBalancer const& balancer_;
        std::span<Real<u.potential> const> potentials_;

        struct ContactStats {
        public:
            Real<u.potential> potDelta;
            Real<u.conductivity> conductivity;

            [[nodiscard]]
            Real<u.force> force() const {
                return potDelta * conductivity;
            }

            [[nodiscard]]
            Real<u.conductivity> derivative() const {
                return -conductivity;
            }
        };

        [[nodiscard]]
        std::vector<Node> const& nodes() const {
            return balancer_.structure().nodes();
        }

        [[nodiscard]]
        std::vector<NodeInfo> const& nodeInfos() const {
            return balancer_.nodeInfos();
        }

        [[nodiscard]]
        ContactStats contactStatsOf(ContactInfo const& contact, Real<u.potential> const localPotential) const {
            Real<u.potential> const pDelta = potentials_[contact.otherIndex()] - localPotential;
            Real<u.conductivity> const conductivity = 1.f / ((pDelta >= 0.f * u.potential) ? contact.rPlus() : contact.rMinus());
            return { pDelta, conductivity };
        }
    };
}
