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

#include <deque>
#include <span>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/LocalContact.hpp>
#include <gustave/utils/IndexRange.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    struct ClusterStructure {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using F1Structure = detail::F1Structure<libCfg>;
        using ClusterIndex = cfg::NodeIndex<libCfg>;
        using ContactIndex = cfg::LinkIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        using ClusterContact = detail::LocalContact<libCfg>;

        class Cluster {
        public:
            [[nodiscard]]
            explicit Cluster(utils::IndexRange<ContactIndex> contactIds, Real<u.force> weight)
                : contactIds_{ contactIds }
                , weight_{ weight }
            {}

            [[nodiscard]]
            utils::IndexRange<ContactIndex> const& contactIds() const {
                return contactIds_;
            }

            [[nodiscard]]
            Real<u.force> weight() const {
                return weight_;
            }

            [[nodiscard]]
            bool operator==(Cluster const&) const = default;
        private:
            utils::IndexRange<ContactIndex> contactIds_;
            Real<u.force> weight_;
        };

        [[nodiscard]]
        static constexpr ClusterIndex invalidClusterId() {
            return std::numeric_limits<ClusterIndex>::max();
        }

        [[nodiscard]]
        explicit ClusterStructure(F1Structure const& fStructure)
            : clusterOfNode_(fStructure.fNodes().size(), invalidClusterId())
        {
            NodeIndex const nodeCount = fStructure.fNodes().size();

            auto numContactsOf = std::vector<ContactIndex>{};
            numContactsOf.reserve(nodeCount);
            for (auto const& fNode : fStructure.fNodes()) {
                numContactsOf.push_back(fNode.contactIds.size());
            }
            for (NodeIndex nodeId = 0; nodeId < nodeCount; ++nodeId) {
                auto const& fNode = fStructure.fNodes()[nodeId];
                if (fNode.isFoundation) {
                    numContactsOf[nodeId] = 0;
                    for (auto const& fContact : fStructure.fContactsOf(nodeId)) {
                        numContactsOf[fContact.otherIndex()] -= 1;
                    }
                }
            }

            auto selectRootId = [&](NodeIndex const& initRootId) {
                NodeIndex result = initRootId;
                if (numContactsOf[initRootId] == 1) {
                    auto const nodeContacts = fStructure.fContactsOf(initRootId);
                    auto remContactIt = std::ranges::find_if(nodeContacts, [&](auto const& contact) {
                        return numContactsOf[contact.otherIndex()] > 0;
                    });
                    assert(remContactIt != nodeContacts.end());
                    result = remContactIt->otherIndex();
                }
                return result;
            };

            auto selectNodes = [&](NodeIndex const rootId, ClusterIndex const clusterId) {
                std::deque<NodeIndex> result;
                auto addNode = [&](std::deque<NodeIndex>& idContainer, NodeIndex newNodeId) {
                    assert(clusterOfNode_[newNodeId] == invalidClusterId());
                    numContactsOf[newNodeId] = 0;
                    clusterOfNode_[newNodeId] = clusterId;
                    idContainer.push_back(newNodeId);
                };
                addNode(result, rootId);
                for (auto const& fContact : fStructure.fContactsOf(rootId)) {
                    NodeIndex const otherId = fContact.otherIndex();
                    if (numContactsOf[otherId] > 0) {
                        addNode(result, otherId);
                    }
                }
                std::deque<NodeIndex> isolatedNodes;
                for (NodeIndex const nId : result) {
                    for (auto const& fContact : fStructure.fContactsOf(nId)) {
                        NodeIndex const otherId = fContact.otherIndex();
                        if (numContactsOf[otherId] > 0) {
                            numContactsOf[otherId] -= 1;
                            if (numContactsOf[otherId] == 0) {
                                addNode(isolatedNodes, otherId);
                            }
                        }
                    }
                }
                result.insert(result.end(), isolatedNodes.begin(), isolatedNodes.end());
                assert(result.size() > 1);
                return result;
            };

            for (NodeIndex optRootId = 0; optRootId < nodeCount; ++optRootId) {
                if (numContactsOf[optRootId] > 0) {
                    NodeIndex const rootId = selectRootId(optRootId);
                    ClusterIndex const clusterId = clusters_.size();
                    std::deque<NodeIndex> nodes = selectNodes(rootId, clusterId);
                    Real<u.force> weight = 0.f * u.force;
                    ContactIndex const startContactIds = ContactIndex(contacts_.size());
                    ContactIndex sizeContactIds = 0;
                    for (NodeIndex const nId : nodes) {
                        weight += fStructure.fNodes()[nId].weight;
                        for (auto const& fContact : fStructure.fContactsOf(nId)) {
                            NodeIndex const otherId = fContact.otherIndex();
                            if (clusterOfNode_[otherId] != clusterId) {
                                sizeContactIds += 1;
                                contacts_.emplace_back(fContact.basicContact(), nId);
                            }
                        }
                    }
                    if (sizeContactIds > 0) {
                        clusters_.emplace_back(utils::IndexRange<ContactIndex>{startContactIds, sizeContactIds}, weight);
                    } else {
                        for (NodeIndex const nId : nodes) {
                            clusterOfNode_[nId] = invalidClusterId();
                        }
                    }
                }
            }
            clusters_.shrink_to_fit();
            contacts_.shrink_to_fit();
        }

        [[nodiscard]]
        std::vector<Cluster> const& clusters() const {
            return clusters_;
        }

        [[nodiscard]]
        std::vector<ClusterIndex> const& clusterOfNode() const {
            return clusterOfNode_;
        }

        [[nodiscard]]
        std::vector<ClusterContact> const& contacts() const {
            return contacts_;
        }

        [[nodiscard]]
        std::span<ClusterContact const> contactsOf(NodeIndex nodeId) const {
            return clusters_[nodeId].contactIds().subSpanOf(contacts_);
        }
    private:
        std::vector<Cluster> clusters_;
        std::vector<ClusterIndex> clusterOfNode_;
        std::vector<ClusterContact> contacts_;
    };
}
