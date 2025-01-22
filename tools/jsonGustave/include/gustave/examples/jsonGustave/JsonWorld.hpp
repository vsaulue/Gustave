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

#include <cstddef>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/jsonWorld/BlockConstructionInfo.hpp>
#include <gustave/examples/jsonGustave/jsonWorld/BlockType.hpp>
#include <gustave/examples/jsonGustave/jsonWorld/Transaction.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave {
    template<core::cGustave G>
    class JsonWorld {
    public:
        using BlockConstructionInfo = jsonWorld::BlockConstructionInfo<G>;
        using BlockType = jsonWorld::BlockType<G>;
        using Transaction = jsonWorld::Transaction<G>;
        using SyncWorld = typename Transaction::SyncWorld;
        using BlockIndex = typename SyncWorld::BlockIndex;
    private:
        struct MatHasher {
            using is_transparent = void;

            [[nodiscard]]
            std::size_t operator()(BlockType const& blockType) const {
                return std::hash<std::string_view>()(blockType.name());
            }

            [[nodiscard]]
            std::size_t operator()(std::string_view name) const {
                return std::hash<std::string_view>()(name);
            }
        };

        struct MatEquals {
            using is_transparent = void;

            [[nodiscard]]
            bool operator()(BlockType const& m1, BlockType const& m2) const {
                return m1.name() == m2.name();
            }

            [[nodiscard]]
            bool operator()(std::string_view name, BlockType const& blockType) const {
                return blockType.name() == name;
            }

            [[nodiscard]]
            bool operator()(BlockType const& blockType, std::string_view name) const {
                return blockType.name() == name;
            }
        };

        template<auto unit>
        using Vector3 = typename G::template Vector3<unit>;

        static constexpr auto u = G::units();
    public:
        using BlockTypes = std::unordered_set<BlockType, MatHasher, MatEquals>;
        using BlockTypeOf = std::unordered_map<BlockIndex, BlockType const*>;

        [[nodiscard]]
        explicit JsonWorld(Vector3<u.length> const& blockSize, Vector3<u.acceleration> const& g)
            : syncWorld_{ newWorld(blockSize, g) }
        {}

        void addBlockType(BlockType const& blockType) {
            auto insertRes = blockTypes_.insert(blockType);
            if (!insertRes.second) {
                std::stringstream msg;
                msg << "Duplicate blockType name: '" << blockType.name() << "'.";
                throw std::invalid_argument(msg.str());
            }
        }

        [[nodiscard]]
        BlockTypes const& blockTypes() const {
            return blockTypes_;
        }

        [[nodiscard]]
        BlockTypeOf const& blockTypeOf() const {
            return blockTypeOf_;
        }

        void update(Transaction const& transaction) {
            for (auto const& blockAndType : transaction.blockTypeOf()) {
                if (!blockTypes_.contains(blockAndType.second)) {
                    std::stringstream msg;
                    msg << "Unknown blockType name: '" << blockAndType.second << "' (block index: " << blockAndType.first << ").";
                    throw std::invalid_argument(msg.str());
                }
            }
            syncWorld_.modify(transaction.syncTransaction());
            for (auto const& blockAndType : transaction.blockTypeOf()) {
                blockTypeOf_[blockAndType.first] = &*blockTypes_.find(blockAndType.second);
            }
        }

        [[nodiscard]]
        SyncWorld const& syncWorld() const {
            return syncWorld_;
        }
    private:
        [[nodiscard]]
        static SyncWorld newWorld(Vector3<u.length> const& blockSize, Vector3<u.acceleration> const& g) {
            using Solver = typename SyncWorld::Solver;
            using Config = typename Solver::Config;
            auto solverConfig = Config{ g, 0.001f, 100000 };
            return SyncWorld{ blockSize, Solver{ solverConfig } };
        }

        SyncWorld syncWorld_;
        BlockTypes blockTypes_;
        BlockTypeOf blockTypeOf_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::JsonWorld<G>> {
    using JsonWorld = gustave::examples::jsonGustave::JsonWorld<G>;

    using BlockConstructionInfo = typename JsonWorld::BlockConstructionInfo;
    using BlockType = typename JsonWorld::BlockType;
    using Transaction = typename JsonWorld::Transaction;

    template<auto unit>
    using Vector3 = typename G::template Vector3<unit>;

    static constexpr auto u = G::units();

    [[nodiscard]]
    static JsonWorld from_json(nlohmann::json const& json) {
        auto const blockSize = json.at("blockSize").get<Vector3<u.length>>();
        auto const g = json.at("g").get<Vector3<u.acceleration>>();
        auto result = JsonWorld{ blockSize, g };
        {
            auto const blockTypes = json.at("blockTypes").get<std::vector<BlockType>>();
            for (auto const& blockType : blockTypes) {
                result.addBlockType(blockType);
            }
        }
        {
            auto const& bTypes = result.blockTypes();
            auto const blockInfos = json.at("blocks").get<std::vector<BlockConstructionInfo>>();
            auto transaction = Transaction{};
            for (auto const& blockInfo : blockInfos) {
                auto const& bTypeIt = bTypes.find(blockInfo.blockTypeName());
                if (bTypeIt == bTypes.end()) {
                    std::stringstream msg;
                    msg << "blockTypeName '" << blockInfo.blockTypeName() << "' of block " << blockInfo.index();
                    msg << " isn't present in 'blockTypes'.";
                    throw std::invalid_argument(msg.str());
                }
                transaction.addBlock(blockInfo.index(), *bTypeIt, blockInfo.isFoundation());
            }
            result.update(transaction);
        }
        return result;
    }
};
