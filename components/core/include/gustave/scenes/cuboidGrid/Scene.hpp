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
#include <gustave/scenes/cuboidGrid/detail/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/SceneStructure.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/PointerHash.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class Scene {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using BlockReference = detail::BlockReference<cfg, true>;
        using ConstBlockReference = detail::BlockReference<cfg, false>;
        using ConstSceneNeighbour = detail::SceneNeighbour<cfg, false>;
        using ConstSceneNeighbours = detail::SceneNeighbours<cfg, false>;
        using Direction = Math3d::BasicDirection;
        using MaxStress = Model::MaxStress<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SceneBlocks = detail::SceneBlocks<cfg>;
        using SceneNeighbour = detail::SceneNeighbour<cfg, true>;
        using SceneNeighbours = detail::SceneNeighbours<cfg, true>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using BlockIndex = BlockPosition;
        using SceneStructure = CuboidGrid::SceneStructure<cfg>;
        using StructureSet = Utils::PointerHash::Set<std::shared_ptr<SceneStructure const>>;
        using Transaction = CuboidGrid::Transaction<cfg>;

        class TransactionResult {
        public:
            using DeletedSet = std::vector<SceneStructure const*>;
            using NewSet = std::vector<std::shared_ptr<SceneStructure const>>;

            [[nodiscard]]
            TransactionResult(NewSet newStructures, DeletedSet deletedStructures)
                : newStructures_{ std::move(newStructures) }
                , deletedStructures_{ std::move(deletedStructures) }
            {}

            [[nodiscard]]
            NewSet const& newStructures() const {
                return newStructures_;
            }

            [[nodiscard]]
            DeletedSet const& deletedStructures() const {
                return deletedStructures_;
            }
        private:
            NewSet newStructures_;
            DeletedSet deletedStructures_;
        };

        [[nodiscard]]
        explicit Scene(Vector3<u.length> const& blockSize)
            : blockSize_{blockSize}
        {
            if (blockSize.x() <= 0.f * u.length) {
                throw std::invalid_argument{ blockSizeError('x', blockSize.x()) };
            }
            if (blockSize.y() <= 0.f * u.length) {
                throw std::invalid_argument{ blockSizeError('y', blockSize.y()) };
            }
            if (blockSize.z() <= 0.f * u.length) {
                throw std::invalid_argument{ blockSizeError('z', blockSize.z()) };
            }
        }

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        TransactionResult modify(Transaction const& transaction) {
            return TransactionRunner::run(*this, transaction);
        }

        [[nodiscard]]
        SceneStructure const* anyStructureContaining(BlockPosition const& position) const {
            ConstBlockReference block = blocks_.find(position);
            if (!block) {
                std::stringstream stream;
                stream << "No block at position " << position << '.';
                throw std::invalid_argument(stream.str());
            }
            if (block.isFoundation()) {
                for (ConstSceneNeighbour const& neighbour : constNeighbours(block.position())) {
                    if (!neighbour.block.isFoundation()) {
                        return neighbour.block.structure();
                    }
                }
                return nullptr;
            } else {
                return block.structure();
            }
        }

        [[nodiscard]]
        SceneBlocks const& blocks() const {
            return blocks_;
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return blockSize_;
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            Vector3<u.length> const& dims = blockSize_;
            Real<u.area> result = 0.f * u.area;
            switch (direction) {
            case Direction::plusX:
            case Direction::minusX:
                result = dims.y() * dims.z();
                break;
            case Direction::plusY:
            case Direction::minusY:
                result = dims.x() * dims.z();
                break;
            case Direction::plusZ:
            case Direction::minusZ:
                result = dims.x() * dims.y();
                break;
            }
            return result;
        }

        [[nodiscard]]
        StructureSet const& structures() const {
            return structures_;
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            Real<u.length> result = 0.f * u.length;
            switch (direction) {
            case Direction::plusX:
            case Direction::minusX:
                result = blockSize_.x();
                break;
            case Direction::plusY:
            case Direction::minusY:
                result = blockSize_.y();
                break;
            case Direction::plusZ:
            case Direction::minusZ:
                result = blockSize_.z();
                break;
            }
            return result;
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
                for (BlockReference root : newRoots) {
                    std::shared_ptr<SceneStructure const> newStruct = scene.generateStructure(root);
                    if (newStruct) {
                        newStructures.emplace_back(std::move(newStruct));
                    }
                }
            }

            void addBlock(BlockConstructionInfo<cfg> const& newInfo) {
                BlockReference ref = scene.blocks_.insert(newInfo);
                assert(ref);
                if (ref.isFoundation()) {
                    for (SceneNeighbour const& neighbour : scene.neighbours(ref)) {
                        declareRoot(neighbour.block);
                    }
                } else {
                    declareRoot(ref);
                    for (ConstSceneNeighbour const& neighbour : scene.constNeighbours(ref)) {
                        removeStructureOf(neighbour.block);
                    }
                }
            }

            void removeBlock(BlockPosition const& deletedPosition) {
                BlockReference deletedBlock = scene.blocks_.find(deletedPosition);
                assert(deletedBlock);
                newRoots.erase(deletedBlock);
                removeStructureOf(deletedBlock);
                for (SceneNeighbour const& neighbour : scene.neighbours(deletedPosition)) {
                    declareRoot(neighbour.block);
                }
                bool isDeleted = scene.blocks_.erase(deletedPosition);
                assert(isDeleted);
            }

            void declareRoot(BlockReference possibleRoot) {
                if (!possibleRoot.isFoundation()) {
                    auto insertResult = newRoots.insert(possibleRoot);
                    if (insertResult.second) {
                        removeStructureOf(possibleRoot);
                    }
                }
            }

            void removeStructureOf(ConstBlockReference block) {
                SceneStructure const* structure = block.structure();
                if (scene.isStructureValid(structure)) {
                    std::shared_ptr<SceneStructure const> oldStructure = scene.removeStructure(structure);
                    if (oldStructure != nullptr) {
                        removedStructures.push_back(std::move(oldStructure));
                    }
                }
            }

            Scene& scene;
            std::unordered_set<BlockReference> newRoots;
            std::vector<std::shared_ptr<SceneStructure const>> newStructures;
            std::vector<std::shared_ptr<SceneStructure const>> removedStructures; // holds shared_ptr to prevent ABA issues.
        };

        void addContact(SceneStructure& structure, ConstBlockReference source, SceneNeighbour const& neighbour) {
            Direction const direction = neighbour.direction;
            NormalizedVector3 const normal = NormalizedVector3::basisVector(direction);
            Real<u.area> const area = contactAreaAlong(direction);
            Real<u.length> const thickness = thicknessAlong(direction);
            MaxStress const maxStress = MaxStress::minResistance(source.maxStress(), neighbour.block.maxStress());
            structure.addContact(source, neighbour.block, normal, area, thickness, maxStress);
        }

        [[nodiscard]]
        static std::string blockSizeError(char coordSymbol, Real<u.length> value) {
            std::stringstream result;
            result << "blocksize." << coordSymbol << " must be strictly positive (passed: " << value << ").";
            return result.str();
        }

        void checkTransaction(Transaction const& transaction) const {
            auto const& deletedBlocks = transaction.deletedBlocks();
            for (BlockPosition const& delPosition : deletedBlocks) {
                if (!blocks_.contains(delPosition)) {
                    std::stringstream stream;
                    stream << "Invalid deletion at " << delPosition << ": block does not exist in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
            for (BlockConstructionInfo<cfg> const& newBlock : transaction.newBlocks()) {
                BlockPosition const& position = newBlock.position();
                if (blocks_.contains(position) && !deletedBlocks.contains(position)) {
                    std::stringstream stream;
                    stream << "Invalid insertion at " << position << ": block already exists in the scene.";
                    throw std::invalid_argument(stream.str());
                }
            }
        }

        [[nodiscard]]
        ConstSceneNeighbours constNeighbours(ConstBlockReference source) const {
            return { blocks_, source.position() };
        }

        [[nodiscard]]
        ConstSceneNeighbours constNeighbours(BlockPosition const& source) const {
            return { blocks_, source };
        }

        std::shared_ptr<SceneStructure const> generateStructure(BlockReference root) {
            assert(!root.isFoundation());
            if (!isStructureValid(root.structure())) {
                auto newStructure = std::make_shared<SceneStructure>(blocks_);
                std::stack<BlockReference> remainingBlocks;
                remainingBlocks.push(root);
                while (!remainingBlocks.empty()) {
                    BlockReference curBlock = remainingBlocks.top();
                    remainingBlocks.pop();
                    assert(!curBlock.isFoundation());
                    if (curBlock.structure() != newStructure.get()) {
                        newStructure->addBlock(curBlock);
                        curBlock.structure() = newStructure.get();
                        for (SceneNeighbour const& neighbour : neighbours(curBlock)) {
                            BlockReference nBlock = neighbour.block;
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
                structures_.emplace(newStructure);
                return newStructure;
            } else {
                return { nullptr };
            }
        }

        [[nodiscard]]
        bool isStructureValid(SceneStructure const* structure) const {
            return structure != nullptr && structures_.contains(structure);
        }

        [[nodiscard]]
        SceneNeighbours neighbours(BlockReference source) {
            return { blocks_, source.position() };
        }

        [[nodiscard]]
        SceneNeighbours neighbours(BlockPosition const& source) {
            return { blocks_, source };
        }

        [[nodiscard]]
        std::shared_ptr<SceneStructure const> removeStructure(SceneStructure const* structure) {
            auto it = structures_.find(structure);
            if (it != structures_.end()) {
                // unordered_set::extract() doesn't support Hash::is_transparent before c++23.
                auto node = structures_.extract(it);
                return std::move(node.value());
            } else {
                return { nullptr };
            }
        }

        Vector3<u.length> blockSize_;
        SceneBlocks blocks_;
        StructureSet structures_;
    };
}
