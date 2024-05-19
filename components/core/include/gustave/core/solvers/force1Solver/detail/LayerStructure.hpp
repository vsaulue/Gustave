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
            Layer()
                : nodeIndices()
                , lowContacts()
                , weight{ 0.f * u.force }
            {}

            [[nodiscard]]
            bool operator==(Layer const&) const = default;

            std::vector<NodeIndex> nodeIndices;
            std::vector<LayerContact> lowContacts;
            Real<u.force> weight;
        };

        [[nodiscard]]
        explicit LayerStructure(F1Structure const& fStructure)
            : reachedCount_{ 0 }
        {
            std::vector<DepthIndex> depthOf(fStructure.fNodes().size(), -1);
            std::vector<NodeIndex> curLayerIndices;
            for (std::size_t nodeId = 0; nodeId < depthOf.size(); ++nodeId) {
                auto const& node = fStructure.structure().nodes()[nodeId];
                if (node.isFoundation) {
                    depthOf[nodeId] = 0;
                    curLayerIndices.push_back(nodeId);
                }
            }
            reachedCount_ += curLayerIndices.size();

            // Skip layer 0
            std::vector<NodeIndex> nextLayerIndices;
            for (NodeIndex localIndex : curLayerIndices) {
                auto const& fNode = fStructure.fNodes()[localIndex];
                for (auto const& fContact : fNode.contacts) {
                    NodeIndex const otherIndex = fContact.otherIndex();
                    if (depthOf[otherIndex] < 0) {
                        depthOf[otherIndex] = 1;
                        nextLayerIndices.push_back(otherIndex);
                    }
                }
            }
            curLayerIndices.swap(nextLayerIndices);
            nextLayerIndices.clear();

            DepthIndex depth = 1;
            while (!curLayerIndices.empty()) {
                reachedCount_ += curLayerIndices.size();
                Layer& newLayer = layers_.emplace_back();
                for (NodeIndex localIndex : curLayerIndices) {
                    auto const& fNode = fStructure.fNodes()[localIndex];
                    newLayer.nodeIndices.push_back(localIndex);
                    newLayer.weight += fNode.weight;
                    for (auto const& fContact : fNode.contacts) {
                        NodeIndex const otherIndex = fContact.otherIndex();
                        DepthIndex const otherDepth = depthOf[otherIndex];
                        if (otherDepth < 0) {
                            depthOf[otherIndex] = depth + 1;
                            nextLayerIndices.push_back(otherIndex);
                        } else if (otherDepth < depth) {
                            newLayer.lowContacts.emplace_back(fContact.basicContact(), localIndex);
                        }
                    }
                }
                curLayerIndices.swap(nextLayerIndices);
                nextLayerIndices.clear();
                ++depth;
            }

            Real<u.force> cumulatedWeight = 0.f * u.force;
            for (auto layerIt = layers_.rbegin(); layerIt != layers_.rend(); ++layerIt) {
                cumulatedWeight += layerIt->weight;
                layerIt->weight = cumulatedWeight;
            }
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
        std::size_t reachedCount_;
        std::vector<Layer> layers_;
    };
}
