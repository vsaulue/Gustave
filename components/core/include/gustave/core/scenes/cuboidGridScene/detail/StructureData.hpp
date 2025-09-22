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
#include <memory>
#include <optional>
#include <stack>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockDataReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/prop/Ptr.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg>
    struct SceneData;

    template<cfg::cLibConfig auto libCfg>
    class StructureData {
    private:
        static constexpr auto u = cfg::units(libCfg);

        using ConstBlockDataReference = detail::BlockDataReference<libCfg, false>;
        using DataNeighbour = detail::DataNeighbour<libCfg, true>;
        using DataNeighbours = detail::DataNeighbours<libCfg, true>;
        using Direction = math3d::BasicDirection;

        using PressureStress = model::PressureStress<libCfg>;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
        using SolverStructure = solvers::Structure<libCfg>;

        using Link = typename SolverStructure::Link;
        using Node = typename SolverStructure::Node;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using BlockDataReference = detail::BlockDataReference<libCfg, true>;
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;
        using SceneData = detail::SceneData<libCfg>;
        using SolverIndices = std::unordered_map<BlockIndex, NodeIndex>;
        using StructureIndex = cfg::StructureIndex<libCfg>;

        [[nodiscard]]
        explicit StructureData(StructureIndex index, SceneData& sceneData, BlockDataReference root)
            : index_{ index }
            , scene_{ &sceneData }
            , solverStructure_{ std::make_shared<SolverStructure>() }
        {
            std::stack<BlockDataReference> remainingBlocks;
            remainingBlocks.push(root);
            while (!remainingBlocks.empty()) {
                BlockDataReference curBlock = remainingBlocks.top();
                remainingBlocks.pop();
                assert(!curBlock.isFoundation());
                if (curBlock.structureId() != index) {
                    declareBlock(curBlock);
                    curBlock.structureId() = index;
                    for (DataNeighbour const& neighbour : DataNeighbours{ scene_->blocks, curBlock.index() }) {
                        BlockDataReference nBlock = neighbour.block;
                        if (nBlock.isFoundation()) {
                            declareBlock(nBlock);
                            addContact(curBlock, neighbour);
                        } else {
                            if (nBlock.structureId() != index) {
                                remainingBlocks.push(nBlock);
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
        bool contains(ConstBlockDataReference block) const {
            return solverIndices_.contains(block.index());
        }

        [[nodiscard]]
        bool contains(BlockIndex const& index) const {
            ConstBlockDataReference block = scene_->blocks.find(index);
            if (block) {
                return contains(block);
            } else {
                return false;
            }
        }

        [[nodiscard]]
        StructureIndex index() const {
            return index_;
        }

        [[nodiscard]]
        bool isValid() const {
            return scene_->structures.contains(this);
        }

        [[nodiscard]]
        SceneData const& sceneData() const {
            return *scene_;
        }

        void setSceneData(SceneData& value) {
            scene_ = &value;
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(ConstBlockDataReference block) const {
            auto const findResult = solverIndices_.find(block.index());
            if (findResult != solverIndices_.end()) {
                return { findResult->second };
            } else {
                return {};
            }
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(BlockIndex const& index) const {
            ConstBlockDataReference const block = scene_->blocks.find(index);
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
        void declareBlock(ConstBlockDataReference block) {
            assert(block);
            auto insertResult = solverIndices_.insert({ block.index(), NodeIndex{0} });
            if (insertResult.second) {
                NodeIndex newIndex = solverStructure_->addNode(Node{ block.mass(), block.isFoundation() });
                insertResult.first->second = newIndex;
            }
        }

        void addContact(BlockDataReference source, DataNeighbour const& neighbour) {
            Direction const direction = neighbour.direction;
            BlockDataReference nBlock = neighbour.block;
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

        LinkIndex addLink(BlockDataReference localNode, BlockDataReference otherNode, Direction direction) {
            NormalizedVector3 const normal = NormalizedVector3::basisVector(direction);
            Real<u.area> const area = scene_->blocks.contactAreaAlong(direction);
            Real<u.length> const thickness = scene_->blocks.thicknessAlong(direction);
            PressureStress const maxStress = PressureStress::minStress(localNode.maxPressureStress(), otherNode.maxPressureStress());
            return solverStructure_->addLink(Link{ indexOf(localNode), indexOf(otherNode), normal, area, thickness, maxStress });
        }

        [[nodiscard]]
        NodeIndex indexOf(ConstBlockDataReference block) const {
            return solverIndices_.at(block.index());
        }

        StructureIndex index_;
        utils::prop::Ptr<SceneData> scene_;
        std::shared_ptr<SolverStructure> solverStructure_;
        SolverIndices solverIndices_;
    };
}
