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
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/common/UserDataTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/InternalLinks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/structureReference/Blocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/structureReference/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UserData_, bool isMutable_>
    class StructureReference {
    private:
        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        friend class StructureReference;

        using BlockData = detail::BlockData<cfg>;
        using ConstBlockDataReference = detail::BlockDataReference<cfg, false>;

        using SceneData = detail::SceneData<cfg, UserData_>;
        using StructureData = SceneData::StructureData;

        template<typename T>
        using PropSharedPtr = utils::PropSharedPtr<isMutable_, T>;

        template<typename T>
        using Prop = utils::Prop<isMutable_, T>;
    public:
        using UDTraits = common::UserDataTraits<UserData_>;

        using BlockReference = cuboidGridScene::BlockReference<cfg, UserData_, false>;
        using ContactReference = cuboidGridScene::ContactReference<cfg, UserData_, false>;
        using SolverStructure = solvers::Structure<cfg>;
        using NodeIndex = cfg::NodeIndex<cfg>;
        using UserDataMember = UDTraits::StructureMember;

        using BlockIndex = typename BlockReference::BlockIndex;
        using ContactIndex = typename ContactReference::ContactIndex;
        using StructureIndex = cfg::StructureIndex<cfg>;

        template<bool mut>
        using Blocks = structureReference::Blocks<cfg, UserData_, mut>;

        template<bool mut>
        using Contacts = structureReference::Contacts<cfg, UserData_, mut>;

        class Links {
        private:
            using InternalLinks = detail::InternalLinks<cfg, UserData_>;
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
                    auto const structId = structure_->index();
                    while (true) {
                        while (linkIndex_ < internalLinks_.size()) {
                            if ((structId == internalLinks_.source().structureId()) || (structId == internalLinks_[linkIndex_].otherBlock.structureId())) {
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
        explicit StructureReference(PropSharedPtr<StructureData> data)
            : data_{ std::move(data) }
            , index_{ initIndex(data_.get()) }
        {}

        [[nodiscard]]
        explicit StructureReference(Prop<SceneData>& scene, StructureIndex index)
            : data_{ scene.structures.findShared(index) }
            , index_{ index }
        {}

        [[nodiscard]]
        explicit StructureReference(utils::NoInit)
            : StructureReference{ nullptr }
        {}

        [[nodiscard]]
        StructureReference(StructureReference const&)
            requires (not isMutable_)
        = default;

        [[nodiscard]]
        StructureReference(meta::cCvRefOf<StructureReference<cfg, UserData_, true>> auto&& v)
            requires (not isMutable_)
            : StructureReference{ std::forward<decltype(v)>(v).asImmutable() }
        {}

        [[nodiscard]]
        StructureReference(StructureReference&)
            requires (isMutable_)
        = default;

        [[nodiscard]]
        StructureReference(StructureReference&&) = default;

        StructureReference& operator=(StructureReference const&)
            requires (not isMutable_)
        = default;

        StructureReference& operator=(StructureReference&)
            requires (isMutable_)
        = default;

        StructureReference& operator=(meta::cCvRefOf<StructureReference<cfg, UserData_, true>> auto&& v)
            requires (not isMutable_)
        {
            *this = std::forward<decltype(v)>(v).asImmutable();
            return *this;
        }

        StructureReference& operator=(StructureReference&&) = default;

        [[nodiscard]]
        StructureReference<cfg, UserData_, false> asImmutable() const& {
            return StructureReference<cfg, UserData_, false>{ data_ };
        }

        [[nodiscard]]
        StructureReference<cfg, UserData_, false> asImmutable() && {
            return StructureReference<cfg, UserData_, false>{ std::move(data_) };
        }

        [[nodiscard]]
        Blocks<true> blocks()
            requires (isMutable_)
        {
            return Blocks<true>{ *data_ };
        }

        [[nodiscard]]
        Blocks<false> blocks() const {
            return Blocks<false>{ *data_ };
        }

        [[nodiscard]]
        Contacts<true> contacts()
            requires (isMutable_)
        {
            return Contacts<true>{ *data_ };
        }

        [[nodiscard]]
        Contacts<false> contacts() const {
            return Contacts<false>{ *data_ };
        }

        [[nodiscard]]
        StructureIndex index() const {
            if (index_ == invalidIndex()) {
                throw invalidError();
            }
            return data_->index();
        }

        [[nodiscard]]
        std::out_of_range invalidError() const {
            std::stringstream msg;
            if (index_ == invalidIndex()) {
                msg << "Invalid structure (invalid index).";
            } else {
                msg << "Invalid structure at index " << index_ << '.';
            }
            return std::out_of_range{ msg.str() };
        }

        [[nodiscard]]
        static constexpr bool isMutable() {
            return isMutable_;
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
            return data_ != nullptr && data_->isValid();
        }

        [[nodiscard]]
        UserDataMember& userData()
            requires (UDTraits::hasStructureUserData() && isMutable_)
        {
            return getUserData(*this);
        }

        [[nodiscard]]
        UserDataMember const& userData() const
            requires (UDTraits::hasStructureUserData())
        {
            return getUserData(*this);
        }

        template<bool rhsMutable>
        [[nodiscard]]
        bool operator==(StructureReference<cfg, UserData_, rhsMutable> const& rhs) const {
            return data_ == rhs.data_;
        }
    private:
        PropSharedPtr<StructureData> data_;
        StructureIndex index_;

        [[nodiscard]]
        static decltype(auto) getUserData(meta::cCvRefOf<StructureReference> auto&& self) {
            if (self.data_ == nullptr) {
                throw self.invalidError();
            }
            return self.data_->userData();
        }

        [[nodiscard]]
        static constexpr StructureIndex invalidIndex() {
            return SceneData::StructureIdGenerator::invalidIndex();
        }

        [[nodiscard]]
        static StructureIndex initIndex(StructureData const* data) {
            if (data == nullptr) {
                return invalidIndex();
            } else {
                return data->index();
            }
        }
    };
}
