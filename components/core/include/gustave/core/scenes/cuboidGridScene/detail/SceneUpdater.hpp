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

#include <memory>
#include <stack>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/Transaction.hpp>
#include <gustave/core/scenes/cuboidGridScene/TransactionResult.hpp>
#include <gustave/utils/IndexRange.hpp>
#include <gustave/utils/prop/Ptr.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_>
    class SceneUpdater {
    public:
        using SceneData = detail::SceneData<libCfg, UD_>;
        using Transaction = cuboidGridScene::Transaction<libCfg>;
        using TransactionResult = cuboidGridScene::TransactionResult<libCfg>;
    private:
        static constexpr auto u = cfg::units(libCfg);

        using BlockConstructionInfo = Transaction::BlockConstructionInfo;
        using BlockData = SceneData::BlockData;
        using ConstDataNeighbours = detail::DataNeighbours<libCfg, UD_, false>;
        using DataNeighbours = detail::DataNeighbours<libCfg, UD_, true>;
        using StructureData = SceneData::StructureData;
        using StructureIndex = StructureData::StructureIndex;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        struct TransactionContext {
            std::unordered_set<BlockData*> newRoots;
            std::vector<StructureIndex> removedStructures;
        };

    public:
        [[nodiscard]]
        explicit SceneUpdater(SceneData& data)
            : data_{ &data }
        {}

        TransactionResult runTransaction(Transaction const& transaction) {
            checkTransaction(transaction);
            TransactionContext ctx;
            for (auto const& delBlockId : transaction.deletedBlocks()) {
                removeBlock(ctx, delBlockId);
            }
            for (auto const& newBlockInfo : transaction.newBlocks()) {
                addBlock(ctx, newBlockInfo);
            }
            auto const newIdStart = data_->structureIdGenerator.readNextIndex();
            for (auto rootPtr : ctx.newRoots) {
                auto& root = *rootPtr;
                assert(!root.isFoundation());
                if (!data_->isStructureIdValid(root.structureId())) {
                    auto const newStructId = data_->structureIdGenerator();
                    auto newStructure = std::make_shared<StructureData>(newStructId, *data_, root);
                    data_->structures.insert(std::move(newStructure));
                }
            }
            auto const newIdEnd = data_->structureIdGenerator.readNextIndex();
            auto const newStructureIds = utils::IndexRange<StructureIndex>{ newIdStart, newIdEnd - newIdStart };
            return TransactionResult{ newStructureIds, std::move(ctx.removedStructures) };
        }
    private:
        void addBlock(TransactionContext& ctx, BlockConstructionInfo const& newInfo) {
            auto& newBlock = data_->blocks.emplace(newInfo, *data_);
            if (newBlock.isFoundation()) {
                for (auto const& neighbour : neighbours(newBlock)) {
                    declareRoot(ctx, neighbour.otherBlock());
                }
            } else {
                declareRoot(ctx, newBlock);
                for (auto const& neighbour : constNeighbours(newBlock)) {
                    removeStructureOf(ctx, neighbour.otherBlock());
                }
            }
        }

        void checkTransaction(Transaction const& transaction) const {
            auto const& deletedBlocks = transaction.deletedBlocks();
            for (BlockIndex const& delIndex : deletedBlocks) {
                if (!data_->blocks.contains(delIndex)) {
                    std::stringstream stream;
                    stream << "Invalid deletion at " << delIndex << ": block does not exist in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
            for (BlockConstructionInfo const& newBlock : transaction.newBlocks()) {
                BlockIndex const& index = newBlock.index();
                if (data_->blocks.contains(index) && !deletedBlocks.contains(index)) {
                    std::stringstream stream;
                    stream << "Invalid insertion at " << index << ": block already exists in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
        }

        [[nodiscard]]
        ConstDataNeighbours constNeighbours(BlockData const& source) const {
            return ConstDataNeighbours{ *data_, source.index() };
        }

        void declareRoot(TransactionContext& ctx, BlockData& possibleRoot) {
            if (!possibleRoot.isFoundation()) {
                auto insertResult = ctx.newRoots.insert(&possibleRoot);
                if (insertResult.second) {
                    removeStructureOf(ctx, possibleRoot);
                }
            }
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockData& source) {
            return DataNeighbours{ *data_, source.index() };
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockIndex const& source) {
            return DataNeighbours{ *data_, source };
        }

        void removeBlock(TransactionContext& ctx, BlockIndex const& deletedIndex) {
            auto& deletedBlock = data_->blocks.at(deletedIndex);
            ctx.newRoots.erase(&deletedBlock);
            removeStructureOf(ctx, deletedBlock);
            for (auto const& neighbour : neighbours(deletedIndex)) {
                declareRoot(ctx, neighbour.otherBlock());
            }
            [[maybe_unused]] bool isDeleted = data_->blocks.erase(deletedIndex);
            assert(isDeleted);
        }

        void removeStructureOf(TransactionContext& ctx, BlockData const& block) {
            auto const structureId = block.structureId();
            if (structureId != data_->structureIdGenerator.invalidIndex()) {
                auto removedStruct = data_->structures.extract(structureId);
                if (removedStruct != nullptr) {
                    removedStruct->invalidate();
                    ctx.removedStructures.push_back(structureId);
                }
            }
        }

        utils::prop::Ptr<SceneData> data_;
    };
}
