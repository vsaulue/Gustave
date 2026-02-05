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

#include <sstream>
#include <stdexcept>
#include <unordered_set>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/utils/getter.hpp>
#include <gustave/utils/HashEquals.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto cfg>
    class Transaction {
    public:
        using BlockConstructionInfo = cuboidGridScene::BlockConstructionInfo<cfg>;
    private:
        using ConstructionHashEquals = utils::HashEquals<BlockConstructionInfo, utils::getter(&BlockConstructionInfo::index)>;
    public:
        using ConstructionSet = ConstructionHashEquals::Set;
        using DeletedSet = std::unordered_set<BlockIndex>;

        [[nodiscard]]
        Transaction() = default;

        void addBlock(BlockConstructionInfo const& newBlock) {
            auto opResult = newBlocks_.insert(newBlock);
            if (!opResult.second) {
                std::stringstream stream;
                stream << "Duplicate insertion at" << newBlock.index() << '.';
                throw std::invalid_argument(stream.str());
            }
        }

        void removeBlock(BlockIndex const& index) {
            deletedBlocks_.insert(index);
        }

        [[nodiscard]]
        ConstructionSet const& newBlocks() const {
            return newBlocks_;
        }

        [[nodiscard]]
        DeletedSet const& deletedBlocks() const {
            return deletedBlocks_;
        }

        void clear() {
            newBlocks_.clear();
            deletedBlocks_.clear();
        }
    private:
        ConstructionSet newBlocks_;
        DeletedSet deletedBlocks_;
    };
}
