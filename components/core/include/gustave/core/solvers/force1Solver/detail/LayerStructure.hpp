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
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/LayerDecomposition.hpp>
#include <gustave/core/solvers/force1Solver/detail/LocalContact.hpp>
#include <gustave/utils/IndexRange.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class LayerStructure {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using F1Structure = detail::F1Structure<libCfg>;
        using LayerDecomposition = detail::LayerDecomposition<libCfg>;

        using ContactIndex = cfg::LinkIndex<libCfg>;
        using LayerIndex = cfg::NodeIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        using F1BasicContact = typename F1Structure::F1Contact::F1BasicContact;

        using LayerContact = detail::LocalContact<libCfg>;

        class Layer {
        public:
            [[nodiscard]]
            explicit Layer(utils::IndexRange<ContactIndex> lowContactIds, LayerIndex lowLayerId, Real<u.force> cumulatedWeight)
                : lowContactIds_{ lowContactIds }
                , lowLayerId_{ lowLayerId }
                , cumulatedWeight_{ cumulatedWeight }
            {
                assert(cumulatedWeight_ >= 0.f * u.force);
            }

            [[nodiscard]]
            Real<u.force> cumulatedWeight() const {
                return cumulatedWeight_;
            }

            [[nodiscard]]
            bool isFoundation() const {
                return lowContactIds_.size() == 0;
            }

            [[nodiscard]]
            utils::IndexRange<ContactIndex> const& lowContactIds() const {
                return lowContactIds_;
            }

            [[nodiscard]]
            LayerIndex lowLayerId() const {
                return lowLayerId_;
            }

            [[nodiscard]]
            bool operator==(Layer const&) const = default;
        private:
            utils::IndexRange<ContactIndex> lowContactIds_;
            LayerIndex lowLayerId_;
            Real<u.force> cumulatedWeight_;
        };

        [[nodiscard]]
        explicit LayerStructure(F1Structure const& fStructure) {
            auto ld = LayerDecomposition{ fStructure };
            reachedCount_ = ld.reachedCount;

            LayerIndex const lastLayerId = ld.decLayers.size() - 1;
            layers_.reserve(ld.decLayers.size());
            lowContacts_.reserve(ld.lowContactsCount);
            ContactIndex startLowContact = 0;
            while (!ld.decLayers.empty()) {
                auto& decLayer = ld.decLayers.back();
                ContactIndex sizeLowContact = 0;
                for (NodeIndex nodeId : decLayer.nodes) {
                    LayerIndex const decLayerId = ld.layerOfNode[nodeId];
                    for (auto const& fContact : fStructure.fContactsOf(nodeId)) {
                        if (ld.layerOfNode[fContact.otherIndex()] > decLayerId) {
                            lowContacts_.emplace_back(fContact.basicContact(), nodeId);
                            sizeLowContact += 1;
                        }
                    }
                }
                LayerIndex const newLayerId = lastLayerId - decLayer.lowLayerId;
                auto const lowContactIds = utils::IndexRange<ContactIndex>{ startLowContact, sizeLowContact };
                layers_.emplace_back(lowContactIds, newLayerId, decLayer.cumulatedWeight);
                startLowContact += sizeLowContact;
                ld.decLayers.pop_back();
            }
            assert(lowContacts_.size() == ld.lowContactsCount);

            layerOfNode_ = std::move(ld.layerOfNode);
            for (LayerIndex& layerId : layerOfNode_) {
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
        std::vector<LayerContact> const& lowContacts() const {
            return lowContacts_;
        }

        [[nodiscard]]
        std::span<LayerContact const> lowContactsOf(NodeIndex const nodeId) const {
            return layers_[nodeId].lowContactIds().subSpanOf(lowContacts_);
        }

        [[nodiscard]]
        std::size_t reachedCount() const {
            return reachedCount_;
        }
    private:
        std::size_t reachedCount_ = 0;
        std::vector<Layer> layers_;
        std::vector<LayerIndex> layerOfNode_;
        std::vector<LayerContact> lowContacts_;
    };
}
