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

#include <stack>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class LayerStructure {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using LayerIndex = cfg::NodeIndex<libCfg>;
        using DepthIndex = std::int64_t;
    public:
        using F1Structure = detail::F1Structure<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        using F1BasicContact = typename F1Structure::F1Contact::F1BasicContact;

        class LayerContact {
        public:
            using ForceStats = typename F1BasicContact::ForceStats;

            [[nodiscard]]
            explicit LayerContact(F1BasicContact const& fContact, NodeIndex localIndex)
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
            bool operator==(LayerContact const&) const = default;
        private:
            F1BasicContact fContact_;
            NodeIndex localIndex_;
        };

        struct Layer {
            [[nodiscard]]
            Layer() = default;

            [[nodiscard]]
            bool isFoundation() const {
                return lowContacts.empty();
            }

            [[nodiscard]]
            bool operator==(Layer const&) const = default;

            std::vector<LayerContact> lowContacts;
            LayerIndex lowLayer = 0;
            Real<u.force> cumulatedWeight = 0.f * u.force;
        };

        [[nodiscard]]
        explicit LayerStructure(F1Structure const& fStructure)
            : layerOfNode_(fStructure.fNodes().size(), 0)
        {
            std::vector<DepthIndex> depthOfNode(fStructure.fNodes().size(), -1);
            std::stack<std::vector<NodeIndex>> nodesAtDepth;

            DepthIndex depth = 0;
            std::vector<NodeIndex> curNodes;
            for (std::size_t nodeId = 0; nodeId < depthOfNode.size(); ++nodeId) {
                auto const& node = fStructure.structure().nodes()[nodeId];
                if (node.isFoundation) {
                    depthOfNode[nodeId] = 0;
                    curNodes.push_back(nodeId);
                }
            }

            std::vector<NodeIndex> nextNodes;
            while (!curNodes.empty()) {
                reachedCount_ += curNodes.size();
                depth += 1;
                for (NodeIndex localIndex : curNodes) {
                    auto const& fNode = fStructure.fNodes()[localIndex];
                    for (auto const& fContact : fNode.contacts) {
                        NodeIndex const otherIndex = fContact.otherIndex();
                        DepthIndex const otherDepth = depthOfNode[otherIndex];
                        if (otherDepth < 0) {
                            depthOfNode[otherIndex] = depth;
                            nextNodes.push_back(otherIndex);
                        }
                    }
                }
                std::vector<NodeIndex>& newDepth = nodesAtDepth.emplace();
                newDepth.swap(curNodes);
                curNodes.swap(nextNodes);
                assert(nextNodes.empty());
            }

            std::vector<bool> placed(fStructure.fNodes().size(), false);
            std::vector<Layer> newLayers;
            while (!nodesAtDepth.empty()) {
                auto const& curDepthNodes = nodesAtDepth.top();
                DepthIndex const depth = nodesAtDepth.size() - 1;
                for (NodeIndex rootId : curDepthNodes) {
                    if (!placed[rootId]) {
                        std::stack<NodeIndex> remainingNodes;
                        auto const layerId = LayerIndex(newLayers.size());
                        Layer& newLayer = newLayers.emplace_back();
                        placed[rootId] = true;
                        layerOfNode_[rootId] = layerId;
                        remainingNodes.push(rootId);
                        while (!remainingNodes.empty()) {
                            NodeIndex localId = remainingNodes.top();
                            remainingNodes.pop();
                            auto const& fNode = fStructure.fNodes()[localId];
                            newLayer.cumulatedWeight += fNode.weight;
                            for (auto const& fContact : fNode.contacts) {
                                NodeIndex const otherId = fContact.otherIndex();
                                DepthIndex const otherDepth = depthOfNode[otherId];
                                if (otherDepth < depth) {
                                    newLayer.lowContacts.emplace_back(fContact.basicContact(), localId);
                                } else if (otherDepth == depth) {
                                    if (!placed[otherId]) {
                                        placed[otherId] = true;
                                        layerOfNode_[otherId] = layerId;
                                        remainingNodes.push(otherId);
                                    }
                                    assert(layerOfNode_[otherId] == layerId);
                                } else {
                                    assert(placed[otherId]);
                                    Layer& otherLayer = newLayers[layerOfNode_[otherId]];
                                    if (otherLayer.lowLayer == 0) {
                                        assert(layerId > 0);
                                        otherLayer.lowLayer = layerId;
                                        newLayer.cumulatedWeight += otherLayer.cumulatedWeight;
                                        for (auto const& otherContact : otherLayer.lowContacts) {
                                            NodeIndex const adjId = otherContact.otherIndex();
                                            if (!placed[adjId]) {
                                                placed[adjId] = true;
                                                layerOfNode_[adjId] = layerId;
                                                remainingNodes.push(adjId);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                nodesAtDepth.pop();
            }

            layers_.reserve(newLayers.size());
            LayerIndex const lastLayerId = newLayers.size() - 1;
            for (auto layerIt = newLayers.rbegin(); layerIt != newLayers.rend(); ++layerIt) {
                if (layerIt->lowLayer > 0) {
                    layerIt->lowLayer = lastLayerId - layerIt->lowLayer;
                }
                layers_.emplace_back(std::move(*layerIt));
            }
            for (auto& layerId : layerOfNode_) {
                layerId = lastLayerId - layerId;
            }
        }

        [[nodiscard]]
        std::vector<LayerIndex> const& layerOfNode() const {
            return layerOfNode_;
        }

        [[nodiscard]]
        std::vector<Layer> const& layers() const {
            return layers_;
        }

        [[nodiscard]]
        std::size_t reachedCount() const {
            return reachedCount_;
        }
    private:
        std::size_t reachedCount_ = 0;
        std::vector<Layer> layers_;
        std::vector<LayerIndex> layerOfNode_;
    };
}
