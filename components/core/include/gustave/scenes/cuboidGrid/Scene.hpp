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
#include <sstream>
#include <stack>
#include <stdexcept>
#include <unordered_set>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/model/MaxStress.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/Blocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/scenes/cuboidGrid/TransactionResult.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class Scene {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using BlockDataReference = detail::BlockDataReference<cfg, true>;
        using ConstBlockDataReference = detail::BlockDataReference<cfg, false>;
        using ConstDataNeighbour = detail::DataNeighbour<cfg, false>;
        using ConstDataNeighbours = detail::DataNeighbours<cfg, false>;
        using Direction = Math3d::BasicDirection;
        using MaxStress = Model::MaxStress<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SceneData = detail::SceneData<cfg>;
        using DataNeighbour = detail::DataNeighbour<cfg, true>;
        using DataNeighbours = detail::DataNeighbours<cfg, true>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using BlockIndex = BlockPosition;
        using BlockReference = CuboidGrid::BlockReference<cfg>;
        using Blocks = CuboidGrid::Blocks<cfg>;
        using StructureData = detail::StructureData<cfg>;
        using StructureSet = Utils::PointerHash::Set<std::shared_ptr<StructureData const>>;
        using Transaction = CuboidGrid::Transaction<cfg>;
        using TransactionResult = CuboidGrid::TransactionResult<cfg>;

        [[nodiscard]]
        explicit Scene(Vector3<u.length> const& blockSize)
            : data_{ blockSize }
        {}

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        TransactionResult modify(Transaction const& transaction) {
            return TransactionRunner::run(*this, transaction);
        }

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ data_ };
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return data_.blocks.blockSize();
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            return data_.blocks.contactAreaAlong(direction);
        }

        [[nodiscard]]
        StructureSet const& structures() const {
            return data_.structures;
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            return data_.blocks.thicknessAlong(direction);
        }
    private:
        class TransactionRunner {
        public:
            static TransactionResult run(Scene& scene, Transaction const& transaction) {
                return TransactionRunner{ scene, transaction }.result();
            }

            [[nodiscard]]
            TransactionResult result() && {
                typename TransactionResult::DeletedSet deletedStructures;
                deletedStructures.reserve(removedStructures.size());
                for (auto const& structure : removedStructures) {
                    deletedStructures.push_back(structure.get());
                }
                return { std::move(newStructures), deletedStructures };
            }
        private:
            TransactionRunner(Scene& scene, Transaction const& transaction)
                : scene{ scene }
            {
                scene.checkTransaction(transaction);
                for (BlockPosition const& delPosition : transaction.deletedBlocks()) {
                    removeBlock(delPosition);
                }
                for (BlockConstructionInfo<cfg> const& newInfo : transaction.newBlocks()) {
                    addBlock(newInfo);
                }
                for (BlockDataReference root : newRoots) {
                    std::shared_ptr<StructureData const> newStruct = scene.generateStructure(root);
                    if (newStruct) {
                        newStructures.emplace_back(std::move(newStruct));
                    }
                }
            }

            void addBlock(BlockConstructionInfo<cfg> const& newInfo) {
                BlockDataReference ref = scene.data_.blocks.insert(newInfo);
                assert(ref);
                if (ref.isFoundation()) {
                    for (DataNeighbour const& neighbour : scene.neighbours(ref)) {
                        declareRoot(neighbour.block);
                    }
                } else {
                    declareRoot(ref);
                    for (ConstDataNeighbour const& neighbour : scene.constNeighbours(ref)) {
                        removeStructureOf(neighbour.block);
                    }
                }
            }

            void removeBlock(BlockPosition const& deletedPosition) {
                BlockDataReference deletedBlock = scene.data_.blocks.find(deletedPosition);
                assert(deletedBlock);
                newRoots.erase(deletedBlock);
                removeStructureOf(deletedBlock);
                for (DataNeighbour const& neighbour : scene.neighbours(deletedPosition)) {
                    declareRoot(neighbour.block);
                }
                bool isDeleted = scene.data_.blocks.erase(deletedPosition);
                assert(isDeleted);
            }

            void declareRoot(BlockDataReference possibleRoot) {
                if (!possibleRoot.isFoundation()) {
                    auto insertResult = newRoots.insert(possibleRoot);
                    if (insertResult.second) {
                        removeStructureOf(possibleRoot);
                    }
                }
            }

            void removeStructureOf(ConstBlockDataReference block) {
                StructureData const* structure = block.structure();
                if (scene.isStructureValid(structure)) {
                    std::shared_ptr<StructureData const> oldStructure = scene.removeStructure(structure);
                    if (oldStructure != nullptr) {
                        removedStructures.push_back(std::move(oldStructure));
                    }
                }
            }

            Scene& scene;
            std::unordered_set<BlockDataReference> newRoots;
            std::vector<std::shared_ptr<StructureData const>> newStructures;
            std::vector<std::shared_ptr<StructureData const>> removedStructures; // holds shared_ptr to prevent ABA issues.
        };

        void addContact(StructureData& structure, ConstBlockDataReference source, DataNeighbour const& neighbour) {
            Direction const direction = neighbour.direction;
            NormalizedVector3 const normal = NormalizedVector3::basisVector(direction);
            Real<u.area> const area = contactAreaAlong(direction);
            Real<u.length> const thickness = thicknessAlong(direction);
            MaxStress const maxStress = MaxStress::minResistance(source.maxStress(), neighbour.block.maxStress());
            structure.addContact(source, neighbour.block, normal, area, thickness, maxStress);
        }

        void checkTransaction(Transaction const& transaction) const {
            auto const& deletedBlocks = transaction.deletedBlocks();
            for (BlockPosition const& delPosition : deletedBlocks) {
                if (!data_.blocks.contains(delPosition)) {
                    std::stringstream stream;
                    stream << "Invalid deletion at " << delPosition << ": block does not exist in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
            for (BlockConstructionInfo<cfg> const& newBlock : transaction.newBlocks()) {
                BlockPosition const& position = newBlock.position();
                if (data_.blocks.contains(position) && !deletedBlocks.contains(position)) {
                    std::stringstream stream;
                    stream << "Invalid insertion at " << position << ": block already exists in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
        }

        [[nodiscard]]
        ConstDataNeighbours constNeighbours(ConstBlockDataReference source) const {
            return { data_.blocks, source.position() };
        }

        [[nodiscard]]
        ConstDataNeighbours constNeighbours(BlockPosition const& source) const {
            return { data_.blocks, source };
        }

        std::shared_ptr<StructureData const> generateStructure(BlockDataReference root) {
            assert(!root.isFoundation());
            if (!isStructureValid(root.structure())) {
                auto newStructure = std::make_shared<StructureData>(data_);
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
                            } else {
                                if (nBlock.structure() != newStructure.get()) {
                                    remainingBlocks.push(nBlock);
                                } else {
                                    addContact(*newStructure, curBlock, neighbour);
                                }
                            }
                        }
                    }
                }
                data_.structures.emplace(newStructure);
                return newStructure;
            } else {
                return { nullptr };
            }
        }

        [[nodiscard]]
        bool isStructureValid(StructureData const* structure) const {
            return structure != nullptr && data_.structures.contains(structure);
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockDataReference source) {
            return { data_.blocks, source.position() };
        }

        [[nodiscard]]
        DataNeighbours neighbours(BlockPosition const& source) {
            return { data_.blocks, source };
        }

        [[nodiscard]]
        std::shared_ptr<StructureData const> removeStructure(StructureData const* structure) {
            auto it = data_.structures.find(structure);
            if (it != data_.structures.end()) {
                // unordered_set::extract() doesn't support Hash::is_transparent before c++23.
                auto node = data_.structures.extract(it);
                return std::move(node.value());
            } else {
                return { nullptr };
            }
        }

        SceneData data_;
    };
}
