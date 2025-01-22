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

#include <cstddef>
#include <limits>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    struct DepthDecomposition {
    public:
        using F1Structure = detail::F1Structure<libCfg>;
        using DepthIndex = cfg::NodeIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        [[nodiscard]]
        explicit DepthDecomposition(F1Structure const& fStructure)
            : depthOfNode(fStructure.fNodes().size(), std::numeric_limits<DepthIndex>::max())
            , reachedCount{ 0 }
        {
            std::size_t const nodeCount = fStructure.fNodes().size();
            std::vector<bool> isNodeReached(nodeCount, false);
            DepthIndex depth = 0;
            std::vector<NodeIndex> curNodes;
            for (std::size_t nodeId = 0; nodeId < nodeCount; ++nodeId) {
                auto const& fNode = fStructure.fNodes()[nodeId];
                if (fNode.isFoundation) {
                    isNodeReached[nodeId] = true;
                    depthOfNode[nodeId] = 0;
                    curNodes.push_back(nodeId);
                }
            }

            std::vector<NodeIndex> nextNodes;
            while (!curNodes.empty()) {
                reachedCount += curNodes.size();
                depth += 1;
                for (NodeIndex localIndex : curNodes) {
                    for (auto const& fContact : fStructure.fContactsOf(localIndex)) {
                        NodeIndex const otherIndex = fContact.otherIndex();
                        if (!isNodeReached[otherIndex]) {
                            isNodeReached[otherIndex] = true;
                            depthOfNode[otherIndex] = depth;
                            nextNodes.push_back(otherIndex);
                        }
                    }
                }
                std::vector<NodeIndex>& newDepth = nodesAtDepth.emplace_back();
                newDepth.swap(curNodes);
                curNodes.swap(nextNodes);
                assert(nextNodes.empty());
            }
        }

        std::vector<DepthIndex> depthOfNode;
        std::vector<std::vector<NodeIndex>> nodesAtDepth;
        std::size_t reachedCount;
    };
}
