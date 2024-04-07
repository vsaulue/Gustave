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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/meta/MutableIf.hpp>
#include <gustave/scenes/cuboidGridScene/detail/DataNeighbour.hpp>
#include <gustave/scenes/cuboidGridScene/detail/IndexNeighbour.hpp>
#include <gustave/scenes/cuboidGridScene/detail/IndexNeighbours.hpp>
#include <gustave/scenes/cuboidGridScene/detail/SceneBlocks.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, bool isMutable_>
    class DataNeighbours {
    private:
        using QualifiedSceneBlocks = meta::MutableIf<isMutable_, SceneBlocks<cfg>>;
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
            DataNeighbour<cfg, isMutable_> const& operator*() const {
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
                    if (BlockDataReference<cfg, isMutable_> neighbour = neighbours_->blocks_.find(indexIterator_->index)) {
                        value_ = { indexIterator_->direction, neighbour };
                        break;
                    }
                    ++indexIterator_;
                }
            }

            DataNeighbours* neighbours_;
            DataNeighbour<cfg, isMutable_> value_;
            IndexIterator indexIterator_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit DataNeighbours(QualifiedSceneBlocks& blocks, BlockIndex const& source)
            : blocks_{ blocks }
            , indices_{ source }
        {}

        [[nodiscard]]
        Iterator begin() {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr utils::EndIterator end() const {
            return {};
        }
    private:
        QualifiedSceneBlocks& blocks_;
        IndexNeighbours indices_;
    };
}
