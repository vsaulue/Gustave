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
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
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
    class StructureReference;

    namespace detail {
        template<Cfg::cLibConfig auto cfg>
        StructureData<cfg> const& structureDataOf(StructureReference<cfg> const&);
    }

    template<Cfg::cLibConfig auto cfg>
    class StructureReference {
    private:
        using BlockReference = CuboidGrid::BlockReference<cfg>;
        using BlockData = detail::BlockData<cfg>;
        using StructureData = detail::StructureData<cfg>;
    public:
        using SolverStructure = Solvers::Structure<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;

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
            BlockReference at(BlockPosition const& position) const {
                auto it = data_->solverIndices().find(position);
                if (it == data_->solverIndices().end()) {
                    std::stringstream msg;
                    msg << "Structure does not contain the block at " << position << '.';
                    throw std::out_of_range(msg.str());
                }
                return BlockReference{ data_->sceneData(), position };
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *data_ };
            }

            [[nodiscard]]
            bool contains(BlockPosition const& position) const {
                return data_->solverIndices().contains(position);
            }

            [[nodiscard]]
            constexpr Utils::EndIterator end() const {
                return {};
            }

            [[nodiscard]]
            std::optional<BlockReference> find(BlockPosition const& position) const {
                auto it = data_->solverIndices().find(position);
                if (it == data_->solverIndices().end()) {
                    return {};
                } else {
                    return BlockReference{ data_->sceneData(), position };
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
        std::optional<NodeIndex> solverIndexOf(BlockPosition const& position) const {
            return data_->solverIndexOf(position);
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
