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
#include <optional>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/solvers/SolverNode.hpp>
#include <gustave/solvers/SolverStructure.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg>
    struct SceneData;

    template<Cfg::cLibConfig auto cfg>
    class StructureData {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using ConstBlockReference = detail::BlockDataReference<cfg, false>;
        using MaxStress = Model::MaxStress<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SceneData = detail::SceneData<cfg>;
        using SolverNode = Solvers::SolverNode<cfg>;
        using SolverStructure = Solvers::SolverStructure<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;
    public:
        using SolverIndices = std::unordered_map<BlockPosition, NodeIndex>;

        [[nodiscard]]
        explicit StructureData(SceneData const& sceneData)
            : sceneData_{ sceneData }
            , solverStructure_{ std::make_shared<SolverStructure>() }
        {}

        void addBlock(ConstBlockReference block) {
            assert(block);
            NodeIndex const newIndex = solverStructure_->nextNodeIndex();
            auto insertResult = solverIndices_.insert({ block.position(), newIndex});
            if (insertResult.second) {
                solverStructure_->addNode({ block.mass(), block.isFoundation() });
            }
        }

        void addContact(ConstBlockReference b1, ConstBlockReference b2, NormalizedVector3 const& normalOnB1,
                        Real<u.area> area, Real<u.length> thickness, MaxStress const& maxConstraints)
        {
            solverStructure_->addLink({ indexOf(b1), indexOf(b2), normalOnB1, area, thickness, maxConstraints });
        }

        [[nodiscard]]
        bool contains(ConstBlockReference block) const {
            return solverIndices_.contains(block.position());
        }

        [[nodiscard]]
        bool contains(BlockPosition const& position) const {
            ConstBlockReference block = sceneData_.blocks.find(position);
            if (block) {
                return contains(block);
            } else {
                return false;
            }
        }

        [[nodiscard]]
        bool isValid() const {
            return sceneData_.structures.contains(this);
        }

        [[nodiscard]]
        SceneData const& sceneData() const {
            return sceneData_;
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(ConstBlockReference block) const {
            auto const findResult = solverIndices_.find(block.position());
            if (findResult != solverIndices_.end()) {
                return { findResult->second };
            } else {
                return {};
            }
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(BlockPosition const& position) const {
            ConstBlockReference const block = sceneData_.blocks.find(position);
            if (block) {
                return solverIndexOf(block);
            } else {
                return {};
            }
        }

        [[nodiscard]]
        SolverIndices const& solverIndices() const {
            return solverIndices_;
        }

        [[nodiscard]]
        SolverStructure const& solverStructure() const {
            return *solverStructure_;
        }

        [[nodiscard]]
        std::shared_ptr<SolverStructure const> solverStructurePtr() const {
            return solverStructure_;
        }
    private:
        [[nodiscard]]
        NodeIndex indexOf(ConstBlockReference block) const {
            return solverIndices_.at(block.position());
        }

        SceneData const& sceneData_;
        std::shared_ptr<SolverStructure> solverStructure_;
        SolverIndices solverIndices_;
    };
}
