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

#include <unordered_map>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockMappedData.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockReference.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg>
    class SceneBlocks {
    public:
        using BlockMap = std::unordered_map<BlockPosition, BlockMappedData<cfg>>;

        [[nodiscard]]
        SceneBlocks() = default;

        [[nodiscard]]
        bool contains(BlockPosition const& position) const {
            return blocks_.contains(position);
        }

        bool erase(BlockPosition const& position) {
            auto res = blocks_.erase(position);
            return res != 0;
        }

        [[nodiscard]]
        BlockReference<cfg,true> find(BlockPosition const& position) {
            return doFind(*this, position);
        }

        [[nodiscard]]
        BlockReference<cfg,false> find(BlockPosition const& position) const {
            return doFind(*this, position);
        }

        BlockReference<cfg, true> insert(BlockConstructionInfo<cfg> const& info) {
            auto it = blocks_.emplace(info.position(), BlockMappedData{ info }).first;
            return { &(*it) };
        }

        [[nodiscard]]
        std::size_t size() const {
            return blocks_.size();
        }
    private:
        [[nodiscard]]
        static auto doFind(auto&& self, BlockPosition const& position) -> decltype(self.find(position)) {
            auto it = self.blocks_.find(position);
            if (it != self.blocks_.end()) {
                return { &(*it) };
            } else {
                return { nullptr };
            }
        }

        BlockMap blocks_;
    };
}
