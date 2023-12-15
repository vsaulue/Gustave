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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/meta/MutableIf.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg, bool isMutable_>
    class DataNeighbours {
    private:
        using QualifiedSceneBlocks = Meta::MutableIf<isMutable_, SceneBlocks<cfg>>;
        using PosIterator = PositionNeighbours::Iterator;

        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : neighbours_{ nullptr }
                , value_{ Utils::NO_INIT }
                , pos_{}
            {}

            [[nodiscard]]
            explicit Enumerator(DataNeighbours& neighbours)
                : neighbours_{ &neighbours }
                , value_{ Utils::NO_INIT }
                , pos_{ neighbours.positions_.begin() }
            {
                next();
            }

            [[nodiscard]]
            bool isEnd() const {
                return pos_ == positions().end();
            }

            void operator++() {
                ++pos_;
                next();
            }

            [[nodiscard]]
            DataNeighbour<cfg, isMutable_> const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return pos_ == other.pos_;
            }
        private:
            [[nodiscard]]
            PositionNeighbours const& positions() const {
                return neighbours_->positions_;
            }

            void next() {
                while (pos_ != positions().end()) {
                    PositionNeighbour const& nPos = *pos_;
                    if (BlockDataReference<cfg, isMutable_> neighbour = neighbours_->blocks_.find(nPos.position)) {
                        value_ = { nPos.direction, neighbour };
                        break;
                    }
                    ++pos_;
                }
            }

            DataNeighbours* neighbours_;
            DataNeighbour<cfg, isMutable_> value_;
            PosIterator pos_;
        };
    public:
        using Iterator = Utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        DataNeighbours(QualifiedSceneBlocks& blocks, BlockPosition const& source)
            : blocks_{ blocks }
            , positions_{ source }
        {}

        [[nodiscard]]
        Iterator begin() {
            return Iterator{ *this };
        }

        [[nodiscard]]
        constexpr Utils::EndIterator end() const {
            return {};
        }
    private:
        QualifiedSceneBlocks& blocks_;
        PositionNeighbours positions_;
    };
}
