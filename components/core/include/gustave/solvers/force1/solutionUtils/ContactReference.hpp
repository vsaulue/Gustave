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
#include <gustave/solvers/force1/detail/SolutionData.hpp>
#include <gustave/solvers/force1/solutionUtils/NodeReference.hpp>
#include <gustave/solvers/Structure.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Solvers::Force1::SolutionUtils {
    template<Cfg::cLibConfig auto cfg>
    class NodeReference;

    template<Cfg::cLibConfig auto cfg>
    class ContactReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using SolutionData = detail::SolutionData<cfg>;
        using Structure = Solvers::Structure<cfg>;

        using LinkIndex = typename Structure::LinkIndex;
        using StructureLink = typename Structure::Link;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using ContactIndex = typename Structure::ContactIndex;
        using NodeIndex = typename Structure::NodeIndex;
        using NodeReference = SolutionUtils::NodeReference<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;

        [[nodiscard]]
        explicit ContactReference(Utils::NoInit)
            : solution_{ nullptr }
            , index_{ 0, false }
        {}

        [[nodiscard]]
        explicit ContactReference(SolutionData const& solution, ContactIndex const& index)
            : solution_{ &solution }
            , index_{ index }
        {}

        [[nodiscard]]
        Real<u.conductivity> compressionConductivity() const {
            return structureLink().compressionConductivity();
        }

        [[nodiscard]]
        Real<u.force> forceCoord() const {
            return solution_->forceRepartition().forceCoordOnContact(index_);
        }

        [[nodiscard]]
        Vector3<u.force> forceVector() const {
            return solution_->forceRepartition().forceVectorOnContact(index_);
        }

        [[nodiscard]]
        ContactIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        NodeReference localNode() const {
            StructureLink const& link = structureLink();
            if (isOnLocalNode()) {
                return NodeReference{ *solution_, link.localNodeId() };
            } else {
                return NodeReference{ *solution_, link.otherNodeId() };
            }
        }

        [[nodiscard]]
        NormalizedVector3 normal() const {
            StructureLink const& link = structureLink();
            if (isOnLocalNode()) {
                return link.normal();
            } else {
                return -link.normal();
            }
        }

        [[nodiscard]]
        ContactReference opposite() const {
            return ContactReference{ *solution_, index_.opposite() };
        }

        [[nodiscard]]
        NodeReference otherNode() const {
            StructureLink const& link = structureLink();
            if (isOnLocalNode()) {
                return NodeReference{ *solution_, link.otherNodeId() };
            } else {
                return NodeReference{ *solution_, link.localNodeId() };
            }
        }

        [[nodiscard]]
        Real<u.conductivity> shearConductivity() const {
            return structureLink().shearConductivity();
        }

        [[nodiscard]]
        Real<u.conductivity> tensileConductivity() const {
            return structureLink().tensileConductivity();
        }

        [[nodiscard]]
        bool operator==(ContactReference const&) const = default;
    private:
        [[nodiscard]]
        bool isOnLocalNode() const {
            return index_.isOnLocalNode;
        }

        [[nodiscard]]
        LinkIndex linkIndex() const {
            return index_.linkIndex;
        }

        [[nodiscard]]
        StructureLink const& structureLink() const {
            return solution_->basis().structure().links()[linkIndex()];
        }

        SolutionData const* solution_;
        ContactIndex index_;
    };
}
