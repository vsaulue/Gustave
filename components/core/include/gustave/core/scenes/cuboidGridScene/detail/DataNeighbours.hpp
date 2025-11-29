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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneBlocks.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_, bool isMut_>
    class DataNeighbours {
    public:
        using Neighbour = DataNeighbour<cfg, UD_, isMut_>;
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        using IndexIterator = IndexNeighbours::Iterator;

        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : neighbours_{ nullptr }
                , value_{ utils::NO_INIT }
                , indexIterator_{}
            {}

            [[nodiscard]]
            explicit Enumerator(DataNeighbours& neighbours)
                : neighbours_{ &neighbours }
                , value_{ utils::NO_INIT }
                , indexIterator_{ neighbours.indices_.begin() }
            {
                next();
            }

            [[nodiscard]]
            bool isEnd() const {
                return indexIterator_ == indices().end();
            }

            void operator++() {
                ++indexIterator_;
                next();
            }

            [[nodiscard]]
            Neighbour const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return indexIterator_ == other.indexIterator_;
            }
        private:
            [[nodiscard]]
            IndexNeighbours const& indices() const {
                return neighbours_->indices_;
            }

            void next() {
                while (indexIterator_ != indices().end()) {
                    if (auto neighbour = neighbours_->blocks_->find(indexIterator_->index)) {
                        value_ = { indexIterator_->direction, neighbour };
                        break;
                    }
                    ++indexIterator_;
                }
            }

            DataNeighbours* neighbours_;
            Neighbour value_;
            IndexIterator indexIterator_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit DataNeighbours(Prop<SceneBlocks<cfg, UD_>>& blocks, BlockIndex const& source)
            : blocks_{ &blocks }
            , indices_{ source }
        {}

        [[nodiscard]]
        Iterator begin() {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }
    private:
        Prop<SceneBlocks<cfg, UD_>>* blocks_;
        IndexNeighbours indices_;
    };
}
