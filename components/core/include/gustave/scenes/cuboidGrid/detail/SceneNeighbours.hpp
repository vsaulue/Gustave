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
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneNeighbour.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg, bool isMutable_>
    class SceneNeighbours {
    public:
        using QualifiedSceneBlocks = Meta::MutableIf<isMutable_, SceneBlocks<cfg>>;

        class EndIterator {
        public:
            [[nodiscard]]
            EndIterator() = default;
        };

        class Iterator {
        public:
            using PosIterator = PositionNeighbours::Iterator;

            [[nodiscard]]
            Iterator(SceneNeighbours& neighbours)
                : neighbours_{ &neighbours }
                , value_{ Utils::NO_INIT }
                , pos_{ neighbours.positions_.begin() }
            {
                next();
            }

            [[nodiscard]]
            bool operator==(EndIterator const&) const {
                return pos_ == positions().end();
            }

            Iterator& operator++() {
                ++pos_;
                next();
                return *this;
            }

            Iterator& operator++(int) {
                Iterator result = *this;
                ++*this;
                return result;
            }

            [[nodiscard]]
            SceneNeighbour<cfg,isMutable_> const& operator*() const {
                return value_;
            }
        private:
            [[nodiscard]]
            PositionNeighbours const& positions() const {
                return neighbours_->positions_;
            }

            void next() {
                while (pos_ != positions().end()) {
                    PositionNeighbour const& nPos = *pos_;
                    if (BlockDataReference<cfg,isMutable_> neighbour = neighbours_->blocks_.find(nPos.position)) {
                        value_ = { nPos.direction, neighbour };
                        break;
                    }
                    ++pos_;
                }
            }

            SceneNeighbours* neighbours_;
            SceneNeighbour<cfg,isMutable_> value_;
            PosIterator pos_;
        };

        [[nodiscard]]
        SceneNeighbours(QualifiedSceneBlocks& blocks, BlockPosition const& source)
            : blocks_{ blocks }
            , positions_{ source }
        {}

        [[nodiscard]]
        Iterator begin() {
            return { *this };
        }

        [[nodiscard]]
        EndIterator end() const {
            return {};
        }
    private:
        QualifiedSceneBlocks& blocks_;
        PositionNeighbours positions_;
    };
}
