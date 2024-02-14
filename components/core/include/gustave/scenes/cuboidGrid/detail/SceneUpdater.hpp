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
#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
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
        using LinkIndex = Cfg::LinkIndex<cfg>;
        using MaxStress = Model::MaxStress<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SceneData = detail::SceneData<cfg>;
        using StructureData = detail::StructureData<cfg>;
        using Transaction = CuboidGrid::Transaction<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        struct TransactionContext {
            std::unordered_set<BlockDataReference> newRoots;
            std::vector<std::shared_ptr<StructureData const>> newStructures;
            std::vector<std::shared_ptr<StructureData const>> removedStructures;
        };
    public:
        struct Result {
            std::vector<std::shared_ptr<StructureData const>> newStructures;
            std::vector<std::shared_ptr<StructureData const>> removedStructures;
        };

        [[nodiscard]]
        explicit SceneUpdater(SceneData& data)
            : data_{ &data }
        {}

        Result runTransaction(Transaction const& transaction) {
            checkTransaction(transaction);
            TransactionContext ctx;
            for (BlockIndex const& delIndex : transaction.deletedBlocks()) {
                removeBlock(ctx, delIndex);
            }
            for (BlockConstructionInfo const& newInfo : transaction.newBlocks()) {
                addBlock(ctx, newInfo);
            }
            for (BlockDataReference root : ctx.newRoots) {
                assert(!root.isFoundation());
                if (!data_->isStructureValid(root.structure())) {
                    auto newStructure = std::make_shared<StructureData const>(*data_, root);
                    data_->structures.emplace(newStructure);
                    ctx.newStructures.emplace_back(std::move(newStructure));
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
            } else {
                declareRoot(ctx, ref);
                for (ConstDataNeighbour const& neighbour : constNeighbours(ref)) {
                    removeStructureOf(ctx, neighbour.block);
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
        ConstDataNeighbours constNeighbours(ConstBlockDataReference source) const {
            return ConstDataNeighbours{ data_->blocks, source.index() };
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
        static Result generateResult(TransactionContext&& ctx) {
            return Result{ std::move(ctx.newStructures), std::move(ctx.removedStructures) };
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockDataReference source) {
            return DataNeighbours{ data_->blocks, source.index() };
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockIndex const& source) {
            return DataNeighbours{ data_->blocks, source };
        }

        void removeBlock(TransactionContext& ctx, BlockIndex const& deletedIndex) {
            BlockDataReference deletedBlock = data_->blocks.find(deletedIndex);
            assert(deletedBlock);
            ctx.newRoots.erase(deletedBlock);
            removeStructureOf(ctx, deletedBlock);
            for (DataNeighbour const& neighbour : neighbours(deletedIndex)) {
                declareRoot(ctx, neighbour.block);
            }
            bool isDeleted = data_->blocks.erase(deletedIndex);
            assert(isDeleted);
        }

        void removeStructureOf(TransactionContext& ctx, ConstBlockDataReference block) {
            StructureData const* structure = block.structure();
            if (structure != nullptr) {
                auto it = data_->structures.find(structure);
                if (it != data_->structures.end()) {
                    // unordered_set::extract() doesn't support Hash::is_transparent before c++23.
                    ctx.removedStructures.push_back(std::move(data_->structures.extract(it).value()));
                }
            }
        }

        Utils::Prop::Ptr<SceneData> data_;
    };
}
