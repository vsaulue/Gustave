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

#include <cassert>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/ContactReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/solvers/Structure.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class BlockReference;

    template<Cfg::cLibConfig auto cfg>
    class ContactReference;

    template<Cfg::cLibConfig auto cfg>
    class StructureReference;

    namespace detail {
        template<Cfg::cLibConfig auto cfg>
        StructureData<cfg> const& structureDataOf(StructureReference<cfg> const&);
    }

    template<Cfg::cLibConfig auto cfg>
    class StructureReference {
    private:
        using BlockData = detail::BlockData<cfg>;
        using ConstBlockDataReference = detail::BlockDataReference<cfg, false>;
        using StructureData = detail::StructureData<cfg>;

        using SceneData = typename StructureData::SceneData;
    public:
        using BlockReference = CuboidGrid::BlockReference<cfg>;
        using ContactReference = CuboidGrid::ContactReference<cfg>;
        using SolverStructure = Solvers::Structure<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;

        using BlockIndex = typename BlockReference::BlockIndex;
        using ContactIndex = typename ContactReference::ContactIndex;

        class Blocks {
        private:
            using DataIterator = typename StructureData::SolverIndices::const_iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : structureData_{ nullptr }
                    , dataIterator_{}
                    , value_{ Utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(StructureData const& structureData)
                    : structureData_{ &structureData }
                    , dataIterator_{ structureData.solverIndices().begin() }
                    , value_{ Utils::NO_INIT }
                {
                    updateValue();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return dataIterator_ == structureData_->solverIndices().end();
                }

                void operator++() {
                    ++dataIterator_;
                    updateValue();
                }

                [[nodiscard]]
                BlockReference const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return dataIterator_ == other.dataIterator_;
                }
            private:
                void updateValue() {
                    if (!isEnd()) {
                        value_ = BlockReference{ structureData_->sceneData(), dataIterator_->first };
                    }
                }

                StructureData const* structureData_;
                DataIterator dataIterator_;
                BlockReference value_;
            };
        public:
            using Iterator = Utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Blocks(StructureData const& data)
                : data_{ &data }
            {}

            [[nodiscard]]
            BlockReference at(BlockIndex const& index) const {
                auto it = data_->solverIndices().find(index);
                if (it == data_->solverIndices().end()) {
                    std::stringstream msg;
                    msg << "Structure does not contain the block at " << index << '.';
                    throw std::out_of_range(msg.str());
                }
                return BlockReference{ data_->sceneData(), index };
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *data_ };
            }

            [[nodiscard]]
            bool contains(BlockIndex const& index) const {
                return data_->solverIndices().contains(index);
            }

            [[nodiscard]]
            constexpr Utils::EndIterator end() const {
                return {};
            }

            [[nodiscard]]
            std::optional<BlockReference> find(BlockIndex const& index) const {
                auto it = data_->solverIndices().find(index);
                if (it == data_->solverIndices().end()) {
                    return {};
                } else {
                    return BlockReference{ data_->sceneData(), index };
                }
            }

            [[nodiscard]]
            std::size_t size() const {
                return data_->solverIndices().size();
            }
        private:
            [[nodiscard]]
            BlockReference blockAt(DataIterator const& dataIterator) const {
                return BlockReference{ data_->sceneData(), dataIterator->first() };
            }

            StructureData const* data_;
        };

        class Contacts {
        public:
            [[nodiscard]]
            explicit Contacts(StructureData const& structure)
                : structure_{ &structure }
            {}

            [[nodiscard]]
            ContactReference at(ContactIndex const& index) const {
                SceneData const& scene = structure_->sceneData();
                ContactReference result{ scene, index };
                BlockIndex const& srcId = index.localBlockIndex();
                ConstBlockDataReference srcBlock = scene.blocks.find(srcId);
                if (srcBlock) {
                    std::optional<BlockIndex> const otherId = srcId.neighbourAlong(index.direction());
                    if (otherId) {
                        ConstBlockDataReference otherBlock = scene.blocks.find(*otherId);
                        if (otherBlock) {
                            if ((structure_ == srcBlock.structure()) || (structure_ == otherBlock.structure())) {
                                return result;
                            }
                        }
                    }
                }
                std::stringstream msg;
                msg << "Structure does not contain the contact at " << index << '.';
                throw std::out_of_range(msg.str());
            }
        private:
            StructureData const* structure_;
        };

        [[nodiscard]]
        explicit StructureReference(std::shared_ptr<StructureData const> data)
            : data_{ std::move(data) }
        {
            assert(data_);
        }

        [[nodiscard]]
        explicit StructureReference(Utils::NoInit)
            : data_{ nullptr }
        {}

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ *data_ };
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ *data_ };
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(BlockIndex const& index) const {
            return data_->solverIndexOf(index);
        }

        [[nodiscard]]
        std::shared_ptr<SolverStructure const> solverStructurePtr() const {
            return data_->solverStructurePtr();
        }

        [[nodiscard]]
        bool isValid() const {
            return data_->isValid();
        }

        [[nodiscard]]
        bool operator==(StructureReference const&) const = default;

        template<Cfg::cLibConfig auto cfg_>
        friend detail::StructureData<cfg_> const& detail::structureDataOf(StructureReference<cfg_> const&);
    private:
        std::shared_ptr<StructureData const> data_;
    public:
        using Hasher = Utils::Hasher<StructureReference, &StructureReference::data_>;
    };

    namespace detail {
        template<Cfg::cLibConfig auto cfg>
        detail::StructureData<cfg> const& structureDataOf(StructureReference<cfg> const& ref) {
            return *ref.data_;
        }
    }
}

template<Gustave::Cfg::cLibConfig auto cfg>
struct std::hash<Gustave::Scenes::CuboidGrid::StructureReference<cfg>> : Gustave::Scenes::CuboidGrid::StructureReference<cfg>::Hasher {};
