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
#include <span>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodeStats.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class ForceRepartition {
    private:
        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using NodeIndex = cfg::NodeIndex<libCfg>;

        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;
    public:
        using F1Structure = detail::F1Structure<libCfg>;
        using Structure = typename F1Structure::Structure;

        using ContactIndex = typename Structure::ContactIndex;
        using ContactStats = typename F1Structure::F1Contact::ForceStats;
        using F1Contact = typename F1Structure::F1Contact;
        using F1Link = typename F1Structure::F1Link;
        using Link = typename Structure::Link;
        using LocalContactIndex = typename F1Link::LocalContactIndex;
        using Node = typename Structure::Node;
        using NodeInfo = typename F1Structure::NodeInfo;
        using NodeStats = detail::NodeStats<libCfg>;

        [[nodiscard]]
        explicit ForceRepartition(F1Structure const& fStructure, std::span<Real<u.potential> const> potentials)
            : fStructure_{ fStructure }
            , potentials_{ potentials }
        {
            assert(potentials_.size() == fStructure.structure().nodes().size());
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
            for (F1Contact const& fContact : info.contacts) {
                ContactStats const contactStats = contactStatsOf(fContact, potential);
                derivative += contactStats.derivative();
                force += contactStats.force();
            }
            return NodeStats{ info, force, derivative };
        }

        [[nodiscard]]
        Real<u.force> forceCoord(NodeIndex to, NodeIndex from) const {
            Real<u.force> result = Real<u.force>::zero();
            NodeInfo const& toInfo = nodeInfos()[to];
            Real<u.potential> const toPotential = potentials_[to];
            for (F1Contact const& fContact : toInfo.contacts) {
                if (fContact.otherIndex() == from) {
                    result += contactStatsOf(fContact, toPotential).force();
                }
            }
            return result;
        }

        [[nodiscard]]
        Real<u.force> forceCoordOnContact(ContactIndex const& index) const {
            Link const& link = fStructure_.structure().links()[index.linkIndex];
            F1Link const& fLink = fStructure_.fLinks()[index.linkIndex];
            NodeIndex const nodeId = index.isOnLocalNode ? link.localNodeId() : link.otherNodeId();
            LocalContactIndex const localContactId = index.isOnLocalNode ? fLink.localContactId : fLink.otherContactId;
            F1Contact const& fContact = fStructure_.nodeInfos()[nodeId].contacts[localContactId];
            ContactStats const stats = contactStatsOf(fContact, potentials_[nodeId]);
            return stats.force();
        }

        [[nodiscard]]
        Vector3<u.force> forceVector(NodeIndex to, NodeIndex from) const {
            return forceCoord(to, from) * fStructure_.normalizedG();
        }

        [[nodiscard]]
        Vector3<u.force> forceVectorOnContact(ContactIndex const& index) const {
            return forceCoordOnContact(index) * fStructure_.normalizedG();
        }
    private:
        F1Structure const& fStructure_;
        std::span<Real<u.potential> const> potentials_;

        [[nodiscard]]
        std::vector<Node> const& nodes() const {
            return fStructure_.structure().nodes();
        }

        [[nodiscard]]
        std::vector<NodeInfo> const& nodeInfos() const {
            return fStructure_.nodeInfos();
        }

        [[nodiscard]]
        ContactStats contactStatsOf(F1Contact const& fContact, Real<u.potential> const localPotential) const {
            return fContact.forceStats(localPotential, potentials_[fContact.otherIndex()]);
        }
    };
}
