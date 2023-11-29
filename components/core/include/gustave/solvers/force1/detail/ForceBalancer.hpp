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

#include <memory>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/Structure.hpp>
#include <gustave/solvers/force1/Config.hpp>
#include <gustave/solvers/force1/detail/ContactInfo.hpp>
#include <gustave/solvers/force1/detail/NodeInfo.hpp>

namespace Gustave::Solvers::Force1::detail {
    template<Cfg::cLibConfig auto cfg>
    class ForceBalancer {
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
        using Structure = Solvers::Structure<cfg>;

        using Config = Force1::Config<cfg>;
        using Contact = typename Structure::Contact;
        using ContactInfo = detail::ContactInfo<cfg>;
        using Node = typename Structure::Node;
        using NodeInfo = detail::NodeInfo<cfg>;

        [[nodiscard]]
        explicit ForceBalancer(Structure const& structure, Config const& config)
            : config_{ &config }
            , structure_{ &structure }
            , normalizedG_{ config_->g() }
        {
            Real<u.acceleration> const gNorm = g().norm();
            nodeInfos_.reserve(nodes().size());
            for (Node const& node : nodes()) {
                nodeInfos_.emplace_back(gNorm * node.mass());
            }
            for (Contact const& contact : structure.contacts()) {
                NodeIndex const id1 = contact.localNodeId();
                NodeIndex const id2 = contact.otherNodeId();

                NormalizedVector3 const& normal = contact.normal();
                Real<u.one> const nComp = normal.dot(normalizedG_);
                Real<u.resistance> const tangentResist = rt.sqrt(1.f - nComp * nComp) / contact.shearConductivity();
                Real<u.resistance> pNormalResist{ Utils::NO_INIT };
                Real<u.resistance> nNormalResist{ Utils::NO_INIT };
                if (nComp <= 0.f) {
                    pNormalResist = -nComp / contact.compressionConductivity();
                    nNormalResist = -nComp / contact.tensileConductivity();
                } else {
                    pNormalResist = nComp / contact.tensileConductivity();
                    nNormalResist = nComp / contact.compressionConductivity();
                }
                Real<u.resistance> const pResist = std::max(pNormalResist, tangentResist);
                Real<u.resistance> const nResist = std::max(nNormalResist, tangentResist);

                nodeInfos_[id1].addContact(id2, pResist, nResist);
                nodeInfos_[id2].addContact(id1, nResist, pResist);
            }
        }

        [[nodiscard]]
        Config const& config() const {
            return *config_;
        }

        [[nodiscard]]
        Vector3<u.acceleration> const& g() const {
            return config_->g();
        }

        [[nodiscard]]
        NormalizedVector3 const& normalizedG() const {
            return normalizedG_;
        }

        [[nodiscard]]
        Structure const& structure() const {
            return *structure_;
        }

        [[nodiscard]]
        std::vector<NodeInfo> const& nodeInfos() const {
            return nodeInfos_;
        }
    private:
        Config const* config_;
        Structure const* structure_;
        std::vector<NodeInfo> nodeInfos_;
        NormalizedVector3 normalizedG_;

        [[nodiscard]]
        std::vector<Node> const& nodes() const {
            return structure_->nodes();
        }
    };
}
