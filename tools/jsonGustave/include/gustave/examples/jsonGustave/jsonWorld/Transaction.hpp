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

#include <unordered_map>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/jsonWorld/BlockType.hpp>

namespace gustave::examples::jsonGustave::jsonWorld {
    template<core::cGustave G>
    class Transaction {
    public:
        using SyncWorld = G::Worlds::SyncWorld;

        using BlockIndex = SyncWorld::BlockIndex;
        using BlockType = jsonWorld::BlockType<G>;
        using SyncTransaction = SyncWorld::Transaction;

        using BlockTypeOf = std::unordered_map<BlockIndex, std::string>;

        void addBlock(BlockIndex const& index, BlockType const& blockType, bool isFoundation) {
            syncTransaction_.addBlock({ index, blockType.maxStress(), blockType.mass(), isFoundation });
            blockTypeOf_.insert({ index, blockType.name() });
        }

        [[nodiscard]]
        SyncTransaction const& syncTransaction() const {
            return syncTransaction_;
        }

        [[nodiscard]]
        BlockTypeOf const& blockTypeOf() const {
            return blockTypeOf_;
        }
    private:
        SyncTransaction syncTransaction_;
        BlockTypeOf blockTypeOf_;
    };
}
