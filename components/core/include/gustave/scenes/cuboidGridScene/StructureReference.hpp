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
#include <gustave/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/scenes/cuboidGridScene/detail/InternalLinks.hpp>
#include <gustave/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/solvers/Structure.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto cfg>
    class BlockReference;

    template<cfg::cLibConfig auto cfg>
    class ContactReference;

    template<cfg::cLibConfig auto cfg>
    class StructureReference;

    namespace detail {
        template<cfg::cLibConfig auto cfg>
        StructureData<cfg> const& structureDataOf(StructureReference<cfg> const&);
    }

    template<cfg::cLibConfig auto cfg>
    class StructureReference {
    private:
        using BlockData = detail::BlockData<cfg>;
        using ConstBlockDataReference = detail::BlockDataReference<cfg, false>;
        using StructureData = detail::StructureData<cfg>;

        using SceneData = typename StructureData::SceneData;
    public:
        using BlockReference = cuboidGridScene::BlockReference<cfg>;
        using ContactReference = cuboidGridScene::ContactReference<cfg>;
        using SolverStructure = solvers::Structure<cfg>;
        using NodeIndex = cfg::NodeIndex<cfg>;

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
                    , value_{ utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(StructureData const& structureData)
                    : structureData_{ &structureData }
                    , dataIterator_{ structureData.solverIndices().begin() }
                    , value_{ utils::NO_INIT }
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
            using Iterator = utils::ForwardIterator<Enumerator>;

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
            constexpr utils::EndIterator end() const {
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

        class Links {
        private:
            using InternalLinks = detail::InternalLinks<cfg>;
            using SolverIndexIterator = typename StructureData::SolverIndices::const_iterator;

            class Enumerator {
            public:
                Enumerator()
                    : structure_{ nullptr }
                    , solverIndexIt_{}
                    , internalLinks_{ utils::NO_INIT }
                    , linkIndex_{ 0 }
                    , value_{ utils::NO_INIT }
                {}

                explicit Enumerator(StructureData const& structure)
                    : structure_{ &structure }
                    , solverIndexIt_{ structure.solverIndices().begin() }
                    , internalLinks_{ utils::NO_INIT }
                    , linkIndex_{ 0 }
                    , value_{ utils::NO_INIT }
                {
                    if (!isEnd()) {
                        updateInternalLinks();
                        next();
                    }
                }

                void operator++() {
                    ++linkIndex_;
                    next();
                }

                [[nodiscard]]
                ContactReference const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool isEnd() const {
                    return solverIndexIt_ == structure_->solverIndices().end();
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return (solverIndexIt_ == other.solverIndexIt_) && (linkIndex_ == other.linkIndex_);
                }
            private:
                void next() {
                    while (true) {
                        while (linkIndex_ < internalLinks_.size()) {
                            if ((structure_ == internalLinks_.source().structure()) || (structure_ == internalLinks_[linkIndex_].otherBlock.structure())) {
                                return updateValue();
                            }
                            ++linkIndex_;
                        }
                        ++solverIndexIt_;
                        if (!isEnd()) {
                            updateInternalLinks();
                            linkIndex_ = 0;
                        } else {
                            return;
                        }
                    }
                }

                void updateInternalLinks() {
                    internalLinks_ = InternalLinks{ structure_->sceneData(), solverIndexIt_->first };
                }

                void updateValue() {
                    ContactIndex index{ solverIndexIt_->first, internalLinks_[linkIndex_].direction };
                    value_ = ContactReference{ structure_->sceneData(), index };
                }

                StructureData const* structure_;
                SolverIndexIterator solverIndexIt_;
                InternalLinks internalLinks_;
                std::size_t linkIndex_;
                ContactReference value_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Links(StructureData const& structure)
                : structure_{ &structure }
            {}

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *structure_ };
            }

            [[nodiscard]]
            utils::EndIterator end() const {
                return {};
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
        explicit StructureReference(utils::NoInit)
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
        Links links() const {
            return Links{ *data_ };
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(BlockIndex const& index) const {
            return data_->solverIndexOf(index);
        }

        [[nodiscard]]
        SolverStructure const& solverStructure() const {
            return data_->solverStructure();
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

        template<cfg::cLibConfig auto libCfg_>
        friend detail::StructureData<libCfg_> const& detail::structureDataOf(StructureReference<libCfg_> const&);
    private:
        std::shared_ptr<StructureData const> data_;
    public:
        using Hasher = utils::Hasher<StructureReference, &StructureReference::data_>;
    };

    namespace detail {
        template<cfg::cLibConfig auto libCfg>
        detail::StructureData<libCfg> const& structureDataOf(StructureReference<libCfg> const& ref) {
            return *ref.data_;
        }
    }
}

template<gustave::cfg::cLibConfig auto libCfg>
struct std::hash<gustave::scenes::cuboidGridScene::StructureReference<libCfg>>
    : public gustave::scenes::cuboidGridScene::StructureReference<libCfg>::Hasher
{};
