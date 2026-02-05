/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <stack>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/common/UserDataTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/prop/Ptr.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_>
    class StructureData {
    public:
        using SceneData = detail::SceneData<libCfg, UD_>;
        using SolverStructure = solvers::Structure<libCfg>;
    private:
        static constexpr auto u = cfg::units(libCfg);

        using DataNeighbour = detail::DataNeighbour<libCfg, UD_, true>;
        using DataNeighbours = detail::DataNeighbours<libCfg, UD_, true>;
        using Direction = math3d::BasicDirection;

        using PressureStress = model::PressureStress<libCfg>;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;

        using Link = SolverStructure::Link;
        using Node = SolverStructure::Node;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using UDTraits = common::UserDataTraits<UD_>;

        using BlockData = SceneData::BlockData;
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;
        using SolverIndices = std::unordered_map<BlockIndex, NodeIndex>;
        using StructureIndex = cfg::StructureIndex<libCfg>;
        using UserDataMember = UDTraits::StructureMember;

        [[nodiscard]]
        static constexpr bool hasUserData() {
            return UDTraits::hasStructureUserData();
        }

        [[nodiscard]]
        explicit StructureData(StructureIndex index, SceneData& sceneData, BlockData& root)
            : index_{ index }
            , scene_{ &sceneData }
            , solverStructure_{ std::make_shared<SolverStructure>() }
            , isValid_{ true }
        {
            std::stack<BlockData*> remainingBlocks;
            remainingBlocks.push(&root);
            while (!remainingBlocks.empty()) {
                auto& curBlock = *remainingBlocks.top();
                remainingBlocks.pop();
                assert(not curBlock.isFoundation());
                if (curBlock.structureId() != index) {
                    declareBlock(curBlock);
                    curBlock.structureId() = index;
                    for (auto const& neighbour : DataNeighbours{ *scene_, curBlock.index() }) {
                        auto& nBlock = neighbour.otherBlock();
                        if (nBlock.isFoundation()) {
                            declareBlock(nBlock);
                            addContact(curBlock, neighbour);
                        } else {
                            if (nBlock.structureId() != index) {
                                remainingBlocks.push(&nBlock);
                            } else {
                                addContact(curBlock, neighbour);
                            }
                        }
                    }
                }
            }
        }

        StructureData(StructureData const&) = delete;
        StructureData& operator=(StructureData const&) = delete;

        [[nodiscard]]
        bool contains(BlockIndex const& index) const {
            return isValid_ && solverIndices_.contains(index);
        }

        [[nodiscard]]
        StructureIndex index() const {
            return index_;
        }

        void invalidate() {
            assert(isValid_);
            isValid_ = false;
            solverStructure_ = nullptr;
            solverIndices_.clear();
        }

        [[nodiscard]]
        bool isValid() const {
            return isValid_;
        }

        [[nodiscard]]
        SceneData& sceneData() {
            return *scene_;
        }

        [[nodiscard]]
        SceneData const& sceneData() const {
            return *scene_;
        }

        void setSceneData(SceneData& value) {
            scene_ = &value;
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(BlockIndex const& index) const {
            if (isValid_) {
                auto const findResult = solverIndices_.find(index);
                if (findResult != solverIndices_.end()) {
                    return { findResult->second };
                }
            }
            return {};
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

        [[nodiscard]]
        UserDataMember const& userData() const
            requires (hasUserData())
        {
            return userData_;
        }

        [[nodiscard]]
        UserDataMember& userData()
            requires (hasUserData())
        {
            return userData_;
        }
    private:
        void declareBlock(BlockData const& block) {
            auto insertResult = solverIndices_.insert({ block.index(), NodeIndex{0} });
            if (insertResult.second) {
                NodeIndex newIndex = solverStructure_->addNode(Node{ block.mass(), block.isFoundation() });
                insertResult.first->second = newIndex;
            }
        }

        void addContact(BlockData& source, DataNeighbour const& neighbour) {
            auto const direction = neighbour.direction();
            auto& nBlock = neighbour.otherBlock();
            switch (direction.id()) {
            case Direction::Id::plusX:
                source.linkIndices().plusX = addLink(source, nBlock, direction);
                break;
            case Direction::Id::plusY:
                source.linkIndices().plusY = addLink(source, nBlock, direction);
                break;
            case Direction::Id::plusZ:
                source.linkIndices().plusZ = addLink(source, nBlock, direction);
                break;
            case Direction::Id::minusX:
                nBlock.linkIndices().plusX = addLink(nBlock, source, direction.opposite());
                break;
            case Direction::Id::minusY:
                nBlock.linkIndices().plusY = addLink(nBlock, source, direction.opposite());
                break;
            case Direction::Id::minusZ:
                nBlock.linkIndices().plusZ = addLink(nBlock, source, direction.opposite());
                break;
            }
        }

        LinkIndex addLink(BlockData const& localNode, BlockData const& otherNode, Direction direction) {
            NormalizedVector3 const normal = NormalizedVector3::basisVector(direction);
            Real<u.area> const area = scene_->contactAreaAlong(direction);
            Real<u.length> const thickness = scene_->thicknessAlong(direction);
            PressureStress const maxStress = PressureStress::minStress(localNode.maxPressureStress(), otherNode.maxPressureStress());
            return solverStructure_->addLink(Link{ indexOf(localNode), indexOf(otherNode), normal, area, thickness, maxStress });
        }

        [[nodiscard]]
        NodeIndex indexOf(BlockData const& block) const {
            return solverIndices_.at(block.index());
        }

        StructureIndex index_;
        utils::prop::Ptr<SceneData> scene_;
        std::shared_ptr<SolverStructure> solverStructure_;
        SolverIndices solverIndices_;

        [[no_unique_address]]
        UserDataMember userData_;
        bool isValid_;
    };
}
