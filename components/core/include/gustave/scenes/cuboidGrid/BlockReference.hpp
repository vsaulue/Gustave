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
#include <cstddef>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/model/MaxStress.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/PositionNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class BlockReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using BlockDataReference = detail::BlockDataReference<cfg, false>;
        using PositionNeighbour = detail::PositionNeighbour;
        using PositionNeighbours = detail::PositionNeighbours;
        using SceneNeighbours = detail::SceneNeighbours<cfg, false>;
        using SceneStructure = CuboidGrid::SceneStructure<cfg>;
        using MaxStress = Model::MaxStress<cfg>;
        using SceneBlocks = detail::SceneBlocks<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using Direction = Math3d::BasicDirection;

        class Neighbour {
        public:
            [[nodiscard]]
            explicit Neighbour(BlockReference const& block, Direction direction)
                : block_{ block }
                , direction_{ direction }
            {}

            [[nodiscard]]
            explicit Neighbour(Utils::NoInit NO_INIT)
                : block_{ NO_INIT }
            {}

            [[nodiscard]]
            BlockReference const& block() const {
                return block_;
            }

            [[nodiscard]]
            Direction direction() const {
                return direction_;
            }

            [[nodiscard]]
            bool operator==(Neighbour const&) const = default;
        private:
            BlockReference block_;
            Direction direction_;
        };

        class Neighbours {
        public:
            class EndIterator {
            public:
                [[nodiscard]]
                constexpr EndIterator() = default;
            };

            class Iterator {
            private:
                using PosIterator = typename PositionNeighbours::Iterator;
            public:
                using difference_type = std::ptrdiff_t;
                using value_type = Neighbour;

                [[nodiscard]]
                Iterator()
                    : neighbours_{ nullptr }
                    , pos_{ nullptr }
                    , value_{ Utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Iterator(Neighbours const& neighbours)
                    : neighbours_{ &neighbours }
                    , pos_{ neighbours.positions_.begin() }
                    , value_{ Utils::NO_INIT }
                {
                    next();
                }

                [[nodiscard]]
                bool operator==(EndIterator const&) const {
                    return pos_ == positions().end();
                }

                [[nodiscard]]
                bool operator==(Iterator const&) const = default;

                Iterator& operator++() {
                    ++pos_;
                    next();
                    return *this;
                }

                Iterator operator++(int) {
                    Iterator result = *this;
                    ++*this;
                    return result;
                }

                [[nodiscard]]
                Neighbour const& operator*() const {
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
                        if (BlockDataReference neighbour = neighbours_->blocks_->find(nPos.position)) {
                            value_ = Neighbour{ BlockReference{ *neighbours_->blocks_, nPos.position }, nPos.direction };
                            break;
                        }
                        ++pos_;
                    }
                }

                Neighbours const* neighbours_;
                PosIterator pos_;
                Neighbour value_;
            };

            [[nodiscard]]
            explicit Neighbours(SceneBlocks const& blocks, BlockPosition const& source)
                : blocks_{ &blocks }
                , positions_ { source }
            {}

            Iterator begin() const {
                return Iterator{ *this };
            }

            EndIterator end() const {
                return {};
            }
        private:
            SceneBlocks const* blocks_;
            PositionNeighbours positions_;
        };

        class Structures {
        private:
            using Values = std::array<SceneStructure const*, 6>;
        public:
            using Iterator = typename Values::const_iterator;

            [[nodiscard]]
            explicit Structures(BlockReference const& block)
                : size_{ 0 }
            {
                if (block.isFoundation()) {
                    for (auto const& neighbour : SceneNeighbours{ *block.sceneBlocks_, block.position_ }) {
                        auto const nBlockData = neighbour.block;
                        if (!nBlockData.isFoundation()) {
                            pushBack(nBlockData.structure());
                        }
                    }
                } else {
                    pushBack(block.data().structure());
                }
            }

            [[nodiscard]]
            SceneStructure const* operator[](std::size_t index) const {
                return sceneStructures_[index];
            }

            [[nodiscard]]
            Iterator begin() const {
                return sceneStructures_.begin();
            }

            [[nodiscard]]
            Iterator end() const {
                return begin() + size_;
            }

            [[nodiscard]]
            std::size_t size() const {
                return size_;
            }
        private:
            void pushBack(SceneStructure const* newValue) {
                assert(newValue != nullptr);
                sceneStructures_[size_] = newValue;
                ++size_;
            }

            Values sceneStructures_;
            std::size_t size_;
        };

        [[nodiscard]]
        explicit BlockReference(SceneBlocks const& sceneBlocks, BlockPosition const& position)
            : sceneBlocks_{ &sceneBlocks }
            , position_{ position }
        {}

        [[nodiscard]]
        explicit BlockReference(Utils::NoInit NO_INIT)
            : position_{ NO_INIT }
        {}

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return sceneBlocks_->blockSize();
        }

        [[nodiscard]]
        BlockPosition const& index() const {
            return position_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return data().isFoundation();
        }

        [[nodiscard]]
        bool isValid() const {
            return sceneBlocks_->contains(position_);
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return data().mass();
        }

        [[nodiscard]]
        MaxStress const& maxStress() const {
            return data().maxStress();
        }

        [[nodiscard]]
        Neighbours neighbours() const {
            return Neighbours{ *sceneBlocks_, position_ };
        }

        [[nodiscard]]
        BlockPosition const& position() const {
            return position_;
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ *this };
        }

        [[nodiscard]]
        bool operator==(BlockReference const&) const = default;
    private:
        SceneBlocks const* sceneBlocks_;
        BlockPosition position_;

        [[nodiscard]]
        BlockDataReference data() const {
            BlockDataReference result = sceneBlocks_->find(position_);
            if (!result) {
                std::stringstream msg;
                msg << "No block at position " << position_ << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }
    };
}
