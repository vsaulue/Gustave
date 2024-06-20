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
#include <gustave/core/solvers/force1Solver/detail/DepthDecomposition.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    struct LayerDecomposition {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using DepthDecomposition = detail::DepthDecomposition<libCfg>;
        using DepthIndex = typename DepthDecomposition::DepthIndex;
    public:
        using F1Structure = detail::F1Structure<libCfg>;
        using LayerIndex = cfg::NodeIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        struct DecLayer {
            std::vector<NodeIndex> nodes;
            Real<u.force> cumulatedWeight = 0.f * u.force;
            LayerIndex lowLayerId = 0;

            [[nodiscard]]
            bool operator==(DecLayer const&) const = default;
        };

        [[nodiscard]]
        explicit LayerDecomposition(F1Structure const& fStructure)
            : layerOfNode(fStructure.fNodes().size(), 0)
        {
            auto dd = DepthDecomposition{ fStructure };
            reachedCount = dd.reachedCount;

            auto const& fNodes = fStructure.fNodes();
            std::vector<bool> isNodePlaced(fNodes.size(), false);
            while (!dd.nodesAtDepth.empty()) {
                DepthIndex const depth = DepthIndex(dd.nodesAtDepth.size()) - 1;
                auto const& curDepthNodes = dd.nodesAtDepth.back();
                for (NodeIndex rootId : curDepthNodes) {
                    if (!isNodePlaced[rootId]) {
                        std::stack<NodeIndex> remainingNodes;
                        LayerIndex const layerId = LayerIndex(decLayers.size());
                        DecLayer& newLayer = decLayers.emplace_back();
                        auto addNodeToLayer = [&](NodeIndex nodeId) {
                            if (!isNodePlaced[nodeId]) {
                                isNodePlaced[nodeId] = true;
                                layerOfNode[nodeId] = layerId;
                                newLayer.nodes.push_back(nodeId);
                                remainingNodes.push(nodeId);
                            }
                        };
                        addNodeToLayer(rootId);
                        while (!remainingNodes.empty()) {
                            NodeIndex localId = remainingNodes.top();
                            auto const& localNode = fNodes[localId];
                            newLayer.cumulatedWeight += localNode.weight;
                            remainingNodes.pop();
                            for (auto const& fContact : localNode.contacts) {
                                NodeIndex const otherId = fContact.otherIndex();
                                DepthIndex const otherDepth = dd.depthOfNode[otherId];
                                if (otherDepth < depth) {
                                    lowContactsCount += 1;
                                } else if (otherDepth == depth) {
                                    addNodeToLayer(otherId);
                                } else {
                                    assert(isNodePlaced[otherId]);
                                    DecLayer& otherLayer = decLayers[layerOfNode[otherId]];
                                    if (otherLayer.lowLayerId == 0) {
                                        assert(layerId > 0);
                                        otherLayer.lowLayerId = layerId;
                                        newLayer.cumulatedWeight += otherLayer.cumulatedWeight;
                                        for (NodeIndex highNodeId : otherLayer.nodes) {
                                            for (auto const& highContact : fNodes[highNodeId].contacts) {
                                                addNodeToLayer(highContact.otherIndex());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                dd.nodesAtDepth.pop_back();
            }
        }

        std::vector<LayerIndex> layerOfNode;
        std::vector<DecLayer> decLayers;
        std::size_t lowContactsCount = 0;
        std::size_t reachedCount = 0;
    };
}
