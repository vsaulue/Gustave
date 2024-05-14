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

#include <memory>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/core/solvers/force1Solver/detail/ContactInfo.hpp>
#include <gustave/core/solvers/force1Solver/detail/LinkInfo.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodeInfo.hpp>
#include <gustave/core/solvers/force1Solver/Config.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class ForceBalancer {
    private:
        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using LinkIndex = cfg::LinkIndex<libCfg>;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        using Rep = typename Real<u.one>::Rep;
        static constexpr auto infConductivity = std::numeric_limits<Rep>::infinity() * u.conductivity;

        struct ConductivityPair {
            Real<u.conductivity> minus;
            Real<u.conductivity> plus;
        };
    public:
        using Structure = solvers::Structure<libCfg>;

        using Config = force1Solver::Config<libCfg>;
        using ContactInfo = detail::ContactInfo<libCfg>;
        using Link = typename Structure::Link;
        using LinkInfo = detail::LinkInfo<libCfg>;
        using LocalContactIndex = typename LinkInfo::LocalContactIndex;
        using Node = typename Structure::Node;
        using NodeInfo = detail::NodeInfo<libCfg>;

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
            auto const& links = structure.links();
            linkInfos_.reserve(links.size());
            for (LinkIndex linkId = 0; linkId < links.size(); ++linkId) {
                Link const& link = links[linkId];
                NodeIndex const id1 = link.localNodeId();
                NodeIndex const id2 = link.otherNodeId();

                NormalizedVector3 const& normal = link.normal();
                Real<u.one> const nComp = normal.dot(normalizedG_);
                Real<u.conductivity> const tangentCond = tangentConductivity(nComp, link);
                ConductivityPair const normalCond = normalConductivities(nComp, link);

                Real<u.conductivity> const pCond = rt.min(normalCond.plus, tangentCond);
                Real<u.conductivity> const nCond = rt.min(normalCond.minus, tangentCond);

                LocalContactIndex contact1 = nodeInfos_[id1].addContact(id2, linkId, pCond, nCond);
                LocalContactIndex contact2 = nodeInfos_[id2].addContact(id1, linkId, nCond, pCond);
                linkInfos_.push_back(LinkInfo{ contact1, contact2 });
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
        std::vector<LinkInfo> const& linkInfos() const {
            return linkInfos_;
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
        [[nodiscard]]
        static ConductivityPair normalConductivities(Real<u.one> const normalComponent, Link const& link) {
            if (normalComponent == 0.f) {
                return { infConductivity, infConductivity };
            }
            Real<u.conductivity> const compression = link.conductivity().compression() / normalComponent;
            Real<u.conductivity> const tensile = link.conductivity().tensile() / normalComponent;
            if (normalComponent < 0.f) {
                return { -tensile, -compression };
            } else {
                return { compression, tensile };
            }
        }

        [[nodiscard]]
        static Real<u.conductivity> tangentConductivity(Real<u.one> normalComponent, Link const& link) {
            Real<u.one> const tComp = rt.sqrt(1.f - normalComponent * normalComponent);
            if (tComp == 0.f) {
                return infConductivity;
            } else {
                return link.conductivity().shear() / tComp;
            }
        }

        Config const* config_;
        Structure const* structure_;
        std::vector<LinkInfo> linkInfos_;
        std::vector<NodeInfo> nodeInfos_;
        NormalizedVector3 normalizedG_;

        [[nodiscard]]
        std::vector<Node> const& nodes() const {
            return structure_->nodes();
        }
    };
}
