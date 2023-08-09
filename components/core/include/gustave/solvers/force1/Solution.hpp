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
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/SolverStructure.hpp>

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

        class ContactInfo {
        public:
            [[nodiscard]]
            ContactInfo(NodeIndex otherIndex, Real<u.resistance> rPlus, Real<u.resistance> rMinus) :
                otherIndex_(otherIndex),
                rMinus_(rMinus),
                rPlus_(rPlus)
            {
                assert(rMinus > 0.f * u.resistance);
                assert(rPlus > 0.f * u.resistance);
            }

            [[nodiscard]]
            NodeIndex otherIndex() const {
                return otherIndex_;
            }

            [[nodiscard]]
            Real<u.resistance> rMinus() const {
                return rMinus_;
            }

            [[nodiscard]]
            Real<u.resistance> rPlus() const {
                return rPlus_;
            }
        private:
            NodeIndex otherIndex_;
            Real<u.resistance> rMinus_;
            Real<u.resistance> rPlus_;
        };

        class NodeInfo {
        public:
            [[nodiscard]]
            NodeInfo(Real<u.force> weight) :
                weight(weight)
            {
                assert(weight > 0.f * u.force);
            }

            void addContact(NodeIndex otherIndex, Real<u.resistance> rPlus, Real<u.resistance> rMinus) {
                contacts.emplace_back(otherIndex, rPlus, rMinus);
            }

            std::vector<ContactInfo> contacts;
            Real<u.force> weight;
        };

        [[nodiscard]]
        Solution(std::shared_ptr<const Basis> basis)
            : basis_(std::move(basis))
            , normalizedG_(basis_->g())
        {
            const Real<u.acceleration> gNorm = g().norm();
            nodeInfos_.reserve(nodes().size());
            for (SolverNode<cfg> const& node : nodes()) {
                nodeInfos_.emplace_back(gNorm * node.mass());
            }
            for (SolverContact<cfg> const& link : structure().links()) {
                const NodeIndex id1 = link.localNodeId();
                const NodeIndex id2 = link.otherNodeId();

                const NormalizedVector3& normal = link.normal();
                const Real<u.one> nComp = normal.dot(normalizedG_);
                Real<u.resistance> tangentResist = rt.sqrt(1.f - nComp * nComp) / link.shearConductivity();
                Real<u.resistance> pNormalResist;
                Real<u.resistance> nNormalResist;
                if (nComp <= 0.f) {
                    pNormalResist = -nComp / link.compressionConductivity();
                    nNormalResist = -nComp / link.tensileConductivity();
                }
                else {
                    pNormalResist = nComp / link.tensileConductivity();
                    nNormalResist = nComp / link.compressionConductivity();
                }
                const Real<u.resistance> pResist = std::max(pNormalResist, tangentResist);
                const Real<u.resistance> nResist = std::max(nNormalResist, tangentResist);

                nodeInfos_[id1].addContact(id2, pResist, nResist);
                nodeInfos_[id2].addContact(id1, nResist, pResist);
            }
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
        std::vector<NodeInfo> const& nodeInfos() const {
            return nodeInfos_;
        }

        [[nodiscard]]
        Real<u.potential> potentialOf(NodeIndex id) const {
            return potentials()[id];
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

        struct NodeStats {
        public:
            [[nodiscard]]
            NodeStats(NodeInfo const& info)
                : info{info}
                , force{info.weight}
                , derivative{0.f * u.conductivity}
            {

            }

            [[nodiscard]]
            Real<u.one> relativeError() const {
                return rt.abs(force / info.weight);
            }

            NodeInfo const& info;
            Real<u.force> force;
            Real<u.conductivity> derivative;
        };

        [[nodiscard]]
        NodeStats statsOf(NodeIndex id) const {
            NodeStats result{ nodeInfos_[id] };

            NodeInfo const& info = nodeInfos_[id];
            Real<u.potential> const potential = potentialOf(id);
            for (ContactInfo const& contactInfo : info.contacts) {
                ContactStats const contactStats = contactStatsOf(contactInfo, potential);
                result.derivative += contactStats.derivative();
                result.force += contactStats.force();
            }
            return result;
        }

        [[nodiscard]]
        Real<u.force> forceCoord(NodeIndex to, NodeIndex from) const {
            Real<u.force> result = Real<u.force>::zero();
            NodeInfo const& toInfo = nodeInfos_[to];
            Real<u.potential> const toPotential = potentialOf(to);
            for (ContactInfo const& contactInfo : toInfo.contacts) {
                if (contactInfo.otherIndex() == from) {
                    result += contactStatsOf(contactInfo, toPotential).force();
                }
            }
            return result;
        }

        [[nodiscard]]
        Vector3<u.force> forceVector(NodeIndex to, NodeIndex from) const {
            return forceCoord(to, from) * normalizedG_;
        }
    private:
        std::shared_ptr<Basis const> basis_;
        std::vector<NodeInfo> nodeInfos_;
        NormalizedVector3 normalizedG_;

        [[nodiscard]]
        std::vector<SolverNode<cfg>> const& nodes() const {
            return basis_->structure().nodes();
        }

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
        ContactStats contactStatsOf(ContactInfo const& contact, Real<u.potential> const localPotential) const {
            Real<u.potential> pDelta = potentialOf(contact.otherIndex()) - localPotential;
            Real<u.conductivity> conductivity = 1.f / ((pDelta >= 0.f * u.potential) ? contact.rPlus() : contact.rMinus());
            return { pDelta, conductivity };
        }
    };
}
