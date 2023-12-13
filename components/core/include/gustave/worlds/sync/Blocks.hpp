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

#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/worlds/sync/BlockReference.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>

namespace Gustave::Worlds::Sync {
    template<Cfg::cLibConfig auto cfg>
    class Blocks {
    private:
        using WorldData = detail::WorldData<cfg>;

        using SceneBlocks = typename WorldData::Scene::Blocks;
    public:
        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using BlockReference = Sync::BlockReference<cfg>;
    private:
        class Enumerator {
        private:
            using SceneIterator = typename SceneBlocks::Iterator;
        public:
            [[nodiscard]]
            Enumerator()
                : blocks_{ nullptr }
            {}

            [[nodiscard]]
            explicit Enumerator(Blocks const& blocks)
                : blocks_{ &blocks }
                , sceneIterator_{ blocks_->sceneBlocks_.begin() }
                , value_{ Utils::NO_INIT }
            {
                updateValue();
            }

            BlockReference const& operator*() const {
                return value_;
            }

            void operator++() {
                ++sceneIterator_;
                updateValue();
            }

            [[nodiscard]]
            bool isEnd() const {
                return sceneIterator_ == blocks_->sceneBlocks_.end();
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return sceneIterator_ == other.sceneIterator_;
            }
        private:
            void updateValue() {
                if (!isEnd()) {
                    value_ = BlockReference{ *blocks_->world_, sceneIterator_->index() };
                }
            }

            Blocks const* blocks_;
            SceneIterator sceneIterator_;
            BlockReference value_;
        };
    public:
        using Iterator = Utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Blocks(WorldData const& data)
            : world_{ &data }
            , sceneBlocks_{ data.scene.blocks() }
        {}

        [[nodiscard]]
        BlockReference at(BlockIndex const& index) const {
            BlockReference result{ *world_, index };
            if (!result.isValid()) {
                std::stringstream msg;
                msg << "No block at index " << index << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr Utils::EndIterator end() const {
            return {};
        }

        [[nodiscard]]
        BlockReference find(BlockIndex const& index) const {
            return BlockReference{ *world_, index };
        }

        [[nodiscard]]
        std::size_t size() const {
            return sceneBlocks_.size();
        }
    private:
        WorldData const* world_;
        SceneBlocks sceneBlocks_;
    };
}
