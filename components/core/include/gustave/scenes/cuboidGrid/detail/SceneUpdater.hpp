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
#include <stack>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/model/MaxStress.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/scenes/cuboidGrid/TransactionResult.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/utils/prop/Ptr.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg>
    class SceneUpdater {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using BlockConstructionInfo = CuboidGrid::BlockConstructionInfo<cfg>;
        using BlockDataReference = detail::BlockDataReference<cfg, true>;
        using ConstBlockDataReference = detail::BlockDataReference<cfg, false>;
        using ConstDataNeighbour = detail::DataNeighbour<cfg, false>;
        using ConstDataNeighbours = detail::DataNeighbours<cfg, false>;
        using DataNeighbour = detail::DataNeighbour<cfg, true>;
        using DataNeighbours = detail::DataNeighbours<cfg, true>;
        using Direction = Math3d::BasicDirection;
        using MaxStress = Model::MaxStress<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SceneData = detail::SceneData<cfg>;
        using StructureData = detail::StructureData<cfg>;
        using Transaction = CuboidGrid::Transaction<cfg>;
        using TransactionResult = CuboidGrid::TransactionResult<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        struct TransactionContext {
            std::unordered_set<BlockDataReference> newRoots;
            std::vector<std::shared_ptr<StructureData const>> newStructures;
            std::vector<std::shared_ptr<StructureData const>> removedStructures;
        };
    public:
        [[nodiscard]]
        explicit SceneUpdater(SceneData& data)
            : data_{ &data }
        {}

        TransactionResult runTransaction(Transaction const& transaction) {
            checkTransaction(transaction);
            TransactionContext ctx;
            for (BlockPosition const& delPosition : transaction.deletedBlocks()) {
                removeBlock(ctx, delPosition);
            }
            for (BlockConstructionInfo const& newInfo : transaction.newBlocks()) {
                addBlock(ctx, newInfo);
            }
            for (BlockDataReference root : ctx.newRoots) {
                std::shared_ptr<StructureData const> newStruct = generateStructure(root);
                if (newStruct) {
                    ctx.newStructures.emplace_back(std::move(newStruct));
                }
            }
            return generateResult(std::move(ctx));
        }
    private:
        void addBlock(TransactionContext& ctx, BlockConstructionInfo const& newInfo) {
            BlockDataReference ref = data_->blocks.insert(newInfo);
            assert(ref);
            if (ref.isFoundation()) {
                for (DataNeighbour const& neighbour : neighbours(ref)) {
                    declareRoot(ctx, neighbour.block);
                }
            }
            else {
                declareRoot(ctx, ref);
                for (ConstDataNeighbour const& neighbour : constNeighbours(ref)) {
                    removeStructureOf(ctx, neighbour.block);
                }
            }
        }

        void addContact(StructureData& structure, ConstBlockDataReference source, DataNeighbour const& neighbour) {
            Direction const direction = neighbour.direction;
            NormalizedVector3 const normal = NormalizedVector3::basisVector(direction);
            Real<u.area> const area = data_->blocks.contactAreaAlong(direction);
            Real<u.length> const thickness = data_->blocks.thicknessAlong(direction);
            MaxStress const maxStress = MaxStress::minResistance(source.maxStress(), neighbour.block.maxStress());
            structure.addContact(source, neighbour.block, normal, area, thickness, maxStress);
        }

        void checkTransaction(Transaction const& transaction) const {
            auto const& deletedBlocks = transaction.deletedBlocks();
            for (BlockPosition const& delPosition : deletedBlocks) {
                if (!data_->blocks.contains(delPosition)) {
                    std::stringstream stream;
                    stream << "Invalid deletion at " << delPosition << ": block does not exist in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
            for (BlockConstructionInfo const& newBlock : transaction.newBlocks()) {
                BlockPosition const& position = newBlock.position();
                if (data_->blocks.contains(position) && !deletedBlocks.contains(position)) {
                    std::stringstream stream;
                    stream << "Invalid insertion at " << position << ": block already exists in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
        }

        [[nodiscard]]
        ConstDataNeighbours constNeighbours(ConstBlockDataReference source) const {
            return { data_->blocks, source.position() };
        }

        [[nodiscard]]
        ConstDataNeighbours constNeighbours(BlockPosition const& source) const {
            return { data_->blocks, source };
        }

        void declareRoot(TransactionContext& ctx, BlockDataReference possibleRoot) {
            if (!possibleRoot.isFoundation()) {
                auto insertResult = ctx.newRoots.insert(possibleRoot);
                if (insertResult.second) {
                    removeStructureOf(ctx, possibleRoot);
                }
            }
        }

        [[nodiscard]]
        std::shared_ptr<StructureData const> generateStructure(BlockDataReference root) {
            assert(!root.isFoundation());
            if (!isStructureValid(root.structure())) {
                auto newStructure = std::make_shared<StructureData>(*data_);
                std::stack<BlockDataReference> remainingBlocks;
                remainingBlocks.push(root);
                while (!remainingBlocks.empty()) {
                    BlockDataReference curBlock = remainingBlocks.top();
                    remainingBlocks.pop();
                    assert(!curBlock.isFoundation());
                    if (curBlock.structure() != newStructure.get()) {
                        newStructure->addBlock(curBlock);
                        curBlock.structure() = newStructure.get();
                        for (DataNeighbour const& neighbour : neighbours(curBlock)) {
                            BlockDataReference nBlock = neighbour.block;
                            if (nBlock.isFoundation()) {
                                newStructure->addBlock(nBlock);
                                addContact(*newStructure, curBlock, neighbour);
                            }
                            else {
                                if (nBlock.structure() != newStructure.get()) {
                                    remainingBlocks.push(nBlock);
                                }
                                else {
                                    addContact(*newStructure, curBlock, neighbour);
                                }
                            }
                        }
                    }
                }
                data_->structures.emplace(newStructure);
                return newStructure;
            }
            else {
                return { nullptr };
            }
        }

        [[nodiscard]]
        static TransactionResult generateResult(TransactionContext&& ctx) {
            typename TransactionResult::DeletedSet deletedStructures;
            deletedStructures.reserve(ctx.removedStructures.size());
            for (auto const& structure : ctx.removedStructures) {
                deletedStructures.push_back(structure.get());
            }
            return TransactionResult{ std::move(ctx.newStructures), deletedStructures };
        }

        [[nodiscard]]
        bool isStructureValid(StructureData const* structure) const {
            return structure != nullptr && data_->structures.contains(structure);
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockDataReference source) {
            return { data_->blocks, source.position() };
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockPosition const& source) {
            return { data_->blocks, source };
        }

        void removeBlock(TransactionContext& ctx, BlockPosition const& deletedPosition) {
            BlockDataReference deletedBlock = data_->blocks.find(deletedPosition);
            assert(deletedBlock);
            ctx.newRoots.erase(deletedBlock);
            removeStructureOf(ctx, deletedBlock);
            for (DataNeighbour const& neighbour : neighbours(deletedPosition)) {
                declareRoot(ctx, neighbour.block);
            }
            bool isDeleted = data_->blocks.erase(deletedPosition);
            assert(isDeleted);
        }

        [[nodiscard]]
        std::shared_ptr<StructureData const> removeStructure(StructureData const* structure) {
            auto it = data_->structures.find(structure);
            if (it != data_->structures.end()) {
                // unordered_set::extract() doesn't support Hash::is_transparent before c++23.
                auto node = data_->structures.extract(it);
                return std::move(node.value());
            }
            else {
                return { nullptr };
            }
        }

        void removeStructureOf(TransactionContext& ctx, ConstBlockDataReference block) {
            StructureData const* structure = block.structure();
            if (isStructureValid(structure)) {
                std::shared_ptr<StructureData const> oldStructure = removeStructure(structure);
                if (oldStructure != nullptr) {
                    ctx.removedStructures.push_back(std::move(oldStructure));
                }
            }
        }

        Utils::Prop::Ptr<SceneData> data_;
    };
}
