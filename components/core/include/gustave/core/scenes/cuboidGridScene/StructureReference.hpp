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
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/structureReference/Blocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/structureReference/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/structureReference/Links.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/solvers/Structure.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_, bool isMut_>
    class StructureReference {
    private:
        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        friend class StructureReference;

        using SceneData = detail::SceneData<cfg, UD_>;
        using StructureData = SceneData::StructureData;

        template<typename T>
        using PropSharedPtr = utils::PropSharedPtr<isMut_, T>;

        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        using UDTraits = common::UserDataTraits<UD_>;
    public:
        using SolverStructure = solvers::Structure<cfg>;
        using NodeIndex = cfg::NodeIndex<cfg>;
        using UserDataMember = UDTraits::StructureMember;

        using BlockIndex = cuboidGridScene::BlockIndex;
        using ContactIndex = cuboidGridScene::ContactIndex;
        using StructureIndex = cfg::StructureIndex<cfg>;

        template<bool mut>
        using BlockReference = cuboidGridScene::BlockReference<cfg, UD_, mut>;

        template<bool mut>
        using Blocks = structureReference::Blocks<cfg, UD_, mut>;

        template<bool mut>
        using Contacts = structureReference::Contacts<cfg, UD_, mut>;

        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<cfg, UD_, mut>;

        template<bool mut>
        using Links = structureReference::Links<cfg, UD_, mut>;

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
            requires (not isMut_)
        = default;

        [[nodiscard]]
        StructureReference(meta::cCvRefOf<StructureReference<cfg, UD_, true>> auto&& v)
            requires (not isMut_)
            : StructureReference{ std::forward<decltype(v)>(v).asImmutable() }
        {}

        [[nodiscard]]
        StructureReference(StructureReference&)
            requires (isMut_)
        = default;

        [[nodiscard]]
        StructureReference(StructureReference&&) = default;

        StructureReference& operator=(StructureReference const&)
            requires (not isMut_)
        = default;

        StructureReference& operator=(StructureReference&)
            requires (isMut_)
        = default;

        StructureReference& operator=(meta::cCvRefOf<StructureReference<cfg, UD_, true>> auto&& v)
            requires (not isMut_)
        {
            *this = std::forward<decltype(v)>(v).asImmutable();
            return *this;
        }

        StructureReference& operator=(StructureReference&&) = default;

        [[nodiscard]]
        StructureReference<cfg, UD_, false> asImmutable() const& {
            return StructureReference<cfg, UD_, false>{ data_ };
        }

        [[nodiscard]]
        StructureReference<cfg, UD_, false> asImmutable() && {
            return StructureReference<cfg, UD_, false>{ std::move(data_) };
        }

        [[nodiscard]]
        Blocks<true> blocks()
            requires (isMut_)
        {
            return Blocks<true>{ *data_ };
        }

        [[nodiscard]]
        Blocks<false> blocks() const {
            return Blocks<false>{ *data_ };
        }

        [[nodiscard]]
        Contacts<true> contacts()
            requires (isMut_)
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
            return isMut_;
        }

        [[nodiscard]]
        Links<true> links()
            requires (isMut_)
        {
            return Links<true>{ *data_ };
        }

        [[nodiscard]]
        Links<false> links() const {
            return Links<false>{ *data_ };
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
            requires (UDTraits::hasStructureUserData() && isMut_)
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
        bool operator==(StructureReference<cfg, UD_, rhsMutable> const& rhs) const {
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
