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
#include <gustave/scenes/cuboidGrid/detail/DataNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/IndexNeighbour.hpp>
#include <gustave/scenes/cuboidGrid/detail/IndexNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class StructureReference;

    template<Cfg::cLibConfig auto cfg>
    class BlockReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using BlockDataReference = detail::BlockDataReference<cfg, false>;
        using DataNeighbours = detail::DataNeighbours<cfg, false>;
        using MaxStress = Model::MaxStress<cfg>;
        using IndexNeighbour = detail::IndexNeighbour;
        using IndexNeighbours = detail::IndexNeighbours;
        using SceneData = detail::SceneData<cfg>;
        using StructureData = detail::StructureData<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using Direction = Math3d::BasicDirection;
        using StructureReference = CuboidGrid::StructureReference<cfg>;

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
        private:
            using IndexIterator = typename IndexNeighbours::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : neighbours_{ nullptr }
                    , index_{ nullptr }
                    , value_{ Utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Neighbours const& neighbours)
                    : neighbours_{ &neighbours }
                    , index_{ neighbours.indices_.begin() }
                    , value_{ Utils::NO_INIT }
                {
                    next();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return index_ == indices().end();
                }

                void operator++() {
                    ++index_;
                    next();
                }

                [[nodiscard]]
                Neighbour const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return index_ == other.index_;
                }
            private:
                [[nodiscard]]
                IndexNeighbours const& indices() const {
                    return neighbours_->indices_;
                }

                void next() {
                    while (index_ != indices().end()) {
                        IndexNeighbour const& idNeighbour = *index_;
                        if (BlockDataReference neighbour = neighbours_->data_->blocks.find(idNeighbour.index)) {
                            value_ = Neighbour{ BlockReference{ *neighbours_->data_, idNeighbour.index }, idNeighbour.direction };
                            break;
                        }
                        ++index_;
                    }
                }

                Neighbours const* neighbours_;
                IndexIterator index_;
                Neighbour value_;
            };
        public:
            using Iterator = Utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Neighbours(SceneData const& data, BlockIndex const& source)
                : data_{ &data }
                , indices_ { source }
            {}

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            constexpr Utils::EndIterator end() const {
                return {};
            }
        private:
            SceneData const* data_;
            IndexNeighbours indices_;
        };

        class Structures {
        private:
            using Values = std::array<StructureReference, 6>;
        public:
            using Iterator = typename Values::const_iterator;

            [[nodiscard]]
            explicit Structures(BlockReference const& block)
                : sceneStructures_{ NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT() }
                , size_{ 0 }
            {
                auto const& structures = block.sceneData_->structures;
                auto addValue = [this, &structures](StructureData const* data) {
                    auto const sharedStructure = *structures.find(data);
                    sceneStructures_[size_] = StructureReference{ std::move(sharedStructure) };
                    ++size_;
                };

                if (block.isFoundation()) {
                    for (auto const& neighbour : DataNeighbours{ block.sceneData_->blocks, block.index_ }) {
                        auto const nBlockData = neighbour.block;
                        if (!nBlockData.isFoundation()) {
                            addValue(nBlockData.structure());
                        }
                    }
                } else {
                    addValue(block.data().structure());
                }
            }

            [[nodiscard]]
            StructureReference const& operator[](std::size_t index) const {
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
            [[nodiscard]]
            static StructureReference NO_INIT() {
                return StructureReference{ Utils::NO_INIT };
            }

            Values sceneStructures_;
            std::size_t size_;
        };

        [[nodiscard]]
        explicit BlockReference(SceneData const& sceneData, BlockIndex const& index)
            : sceneData_{ &sceneData }
            , index_{ index }
        {}

        [[nodiscard]]
        explicit BlockReference(Utils::NoInit NO_INIT)
            : index_{ NO_INIT }
        {}

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return sceneData_->blocks.blockSize();
        }

        [[nodiscard]]
        BlockIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return data().isFoundation();
        }

        [[nodiscard]]
        bool isValid() const {
            return sceneData_->blocks.contains(index_);
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
            return Neighbours{ *sceneData_, index_ };
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ *this };
        }

        [[nodiscard]]
        bool operator==(BlockReference const&) const = default;
    private:
        SceneData const* sceneData_;
        BlockIndex index_;

        [[nodiscard]]
        BlockDataReference data() const {
            BlockDataReference result = sceneData_->blocks.find(index_);
            if (!result) {
                std::stringstream msg;
                msg << "No block at index " << index_ << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }
    };
}
