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

#include <cassert>
#include <cstddef>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_>
    class BlockReference {
    private:
        static constexpr auto u = cfg::units(libCfg);

        using BlockDataReference = detail::BlockDataReference<libCfg, false>;
        using DataNeighbours = detail::DataNeighbours<libCfg, false>;
        using IndexNeighbour = detail::IndexNeighbour;
        using IndexNeighbours = detail::IndexNeighbours;
        using SceneData = detail::SceneData<libCfg, UserData_>;
        using StructureData = SceneData::StructureData;
        using StructureIndex = StructureData::StructureIndex;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;
    public:
        using BlockIndex = cuboidGridScene::BlockIndex;
        using Direction = math3d::BasicDirection;
        using ContactReference = cuboidGridScene::ContactReference<libCfg, UserData_>;
        using PressureStress = model::PressureStress<libCfg>;
        using StructureReference = cuboidGridScene::StructureReference<libCfg, UserData_>;

        class Contacts {
        private:
            using Values = std::array<Direction, 6>;
            using DirIterator = Values::const_iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : contacts_{ nullptr }
                    , value_ { utils::NO_INIT }
                    , state_{ 6 }
                {}

                [[nodiscard]]
                explicit Enumerator(Contacts const& contacts)
                    : contacts_{ &contacts }
                    , value_{ utils::NO_INIT }
                    , state_{ 0 }
                {
                    next();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return state_ >= 6;
                }

                void operator++() {
                    ++state_;
                    next();
                }

                [[nodiscard]]
                ContactReference const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return (contacts_ == other.contacts_) && (state_ == other.state_);
                }
            private:
                void next() {
                    while (!isEnd()) {
                        value_ = contacts_->alongUnchecked(static_cast<Direction::Id>(state_));
                        if (value_.isValid()) {
                            return;
                        }
                        state_++;
                    }
                }

                Contacts const* contacts_;
                ContactReference value_;
                int state_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Contacts(BlockReference const& source)
                : source_{ source }
            {}

            [[nodiscard]]
            ContactReference along(Direction direction) const {
                ContactReference result = alongUnchecked(direction);
                if (!result.isValid()) {
                    throw std::out_of_range(result.invalidMessage());
                }
                return result;
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            utils::EndIterator end() const {
                return {};
            }
        private:
            [[nodiscard]]
            ContactReference alongUnchecked(Direction direction) const {
                return ContactReference{ *source_.sceneData_, ContactIndex{ source_.index_, direction } };
            }

            BlockReference source_;
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
                auto addValue = [&](StructureIndex structId) {
                    if (!contains(structId)) {
                        auto sharedStructure = *structures.find(structId);
                        sceneStructures_[size_] = StructureReference{ std::move(sharedStructure) };
                        ++size_;
                    }
                };

                if (block.isFoundation()) {
                    for (auto const& neighbour : DataNeighbours{ block.sceneData_->blocks, block.index_ }) {
                        auto const nBlockData = neighbour.block;
                        if (!nBlockData.isFoundation()) {
                            addValue(nBlockData.structureId());
                        }
                    }
                } else {
                    addValue(block.data().structureId());
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
                return StructureReference{ utils::NO_INIT };
            }

            [[nodiscard]]
            bool contains(StructureIndex structId) const {
                for (std::size_t id = 0; id < size_; ++id) {
                    if (structId == sceneStructures_[id].index()) {
                        return true;
                    }
                }
                return false;
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
        explicit BlockReference(utils::NoInit NO_INIT)
            : index_{ NO_INIT }
        {}

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return sceneData_->blocks.blockSize();
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ *this };
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
        PressureStress const& maxPressureStress() const {
            return data().maxPressureStress();
        }

        [[nodiscard]]
        Vector3<u.length> position() const {
            using Rep = typename Real<u.length>::Rep;
            Vector3<u.length> const& bSize = blockSize();
            Real<u.length> const x = Rep(index_.x) * bSize.x();
            Real<u.length> const y = Rep(index_.y) * bSize.y();
            Real<u.length> const z = Rep(index_.z) * bSize.z();
            return Vector3<u.length>{ x, y, z };
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
