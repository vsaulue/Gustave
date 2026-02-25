/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/common/UserDataTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/utils/Prop.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_, bool isMut_>
    class ContactReference {
    private:
        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        friend class ContactReference;

        using UDTraits = common::UserDataTraits<UD_>;

        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        template<typename T>
        using PropSharedPtr = utils::PropSharedPtr<isMut_, T>;

        static constexpr auto u = cfg::units(libCfg);

        using SceneData = detail::SceneData<libCfg, UD_>;

        using BlockData = SceneData::BlockData;
        using StructureData = SceneData::StructureData;
        using StructureIndex = SceneData::StructureIndex;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using AsImmutable = ContactReference<libCfg, UD_, false>;
        using BlockIndex = cuboidGridScene::BlockIndex;
        using CommonUserDataMember = UDTraits::CommonMember;
        using ContactIndex = cuboidGridScene::ContactIndex;
        using Direction = ContactIndex::Direction;
        using PressureStress = model::PressureStress<libCfg>;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
        using SolverContactIndex = solvers::Structure<libCfg>::ContactIndex;

        template<bool mut>
        using BlockReference = cuboidGridScene::BlockReference<libCfg, UD_, mut>;

        template<bool mut>
        using StructureReference = cuboidGridScene::StructureReference<libCfg, UD_, mut>;

        [[nodiscard]]
        explicit ContactReference(utils::NoInit NO_INIT)
            : structure_{ nullptr }
            , localBlock_{ nullptr }
            , otherBlock_{ nullptr }
            , index_{ NO_INIT }
        {}

        [[nodiscard]]
        explicit ContactReference(Prop<SceneData>& scene, ContactIndex const& index)
            : structure_{ nullptr }
            , localBlock_{ scene.blocks.find(index.localBlockIndex()) }
            , otherBlock_{ nullptr }
            , index_{ index }
        {
            if (!localBlock_) {
                return;
            }
            if (auto const otherBlockId = index.otherBlockIndex()) {
                otherBlock_ = scene.blocks.find(*otherBlockId);
                if (!otherBlock_) {
                    return;
                }
                if (!localBlock_->isFoundation()) {
                    structure_ = scene.structures.atShared(localBlock_->structureId());
                } else if (!otherBlock_->isFoundation()) {
                    structure_ = scene.structures.atShared(otherBlock_->structureId());
                }
            }
        }

        [[nodiscard]]
        ContactReference(ContactReference&)
            requires (isMut_)
        = default;

        [[nodiscard]]
        ContactReference(ContactReference const&)
            requires (not isMut_)
        = default;

        [[nodiscard]]
        ContactReference(meta::cCvRefOf<ContactReference<libCfg, UD_, true>> auto&& other)
            requires (not isMut_)
            : ContactReference{ std::forward<decltype(other)>(other).asImmutable() }
        {}

        [[nodiscard]]
        ContactReference(ContactReference&&) = default;

        ContactReference& operator=(ContactReference&)
            requires (isMut_)
        = default;

        ContactReference& operator=(ContactReference const&)
            requires (not isMut_)
        = default;

        ContactReference& operator=(meta::cCvRefOf<ContactReference<libCfg, UD_, true>> auto&& rhs)
            requires (not isMut_)
        {
            *this = std::forward<decltype(rhs)>(rhs).asImmutable();
            return *this;
        }

        ContactReference& operator=(ContactReference&&) = default;

        [[nodiscard]]
        Real<u.area> area() const {
            if (!isValid()) {
                throw invalidError();
            }
            return structure_->sceneData().contactAreaAlong(index_.direction());
        }

        [[nodiscard]]
        AsImmutable asImmutable() const
            requires (isMut_)
        {
            return AsImmutable{ structure_->sceneData(), index_ };
        }

        [[nodiscard]]
        CommonUserDataMember& commonUserData()
            requires (isMut_&& UDTraits::hasCommonUserData())
        {
            return doCommonUserData(*this);
        }

        [[nodiscard]]
        CommonUserDataMember const& commonUserData() const
            requires (UDTraits::hasCommonUserData())
        {
            return doCommonUserData(*this);
        }

        [[nodiscard]]
        ContactIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        std::out_of_range invalidError() const {
            std::stringstream msg;
            msg << "Invalid contact at index " << index_ << '.';
            return std::out_of_range(msg.str());
        }

        [[nodiscard]]
        bool isValid() const {
            return structure_ && structure_->isValid();
        }

        [[nodiscard]]
        BlockReference<true> localBlock()
            requires (isMut_)
        {
            return doLocalBlock(*this);
        }

        [[nodiscard]]
        BlockReference<false> localBlock() const {
            return doLocalBlock(*this);
        }

        [[nodiscard]]
        PressureStress maxPressureStress() const {
            if (!isValid()) {
                throw invalidError();
            }
            return PressureStress::minStress(localBlock_->maxPressureStress(), otherBlock_->maxPressureStress());
        }

        [[nodiscard]]
        NormalizedVector3 normal() const {
            if (not isValid()) {
                throw invalidError();
            }
            return NormalizedVector3::basisVector(index_.direction());
        }

        [[nodiscard]]
        ContactReference opposite()
            requires (isMut_)
        {
            return doOpposite(*this);
        }

        [[nodiscard]]
        AsImmutable opposite() const {
            return doOpposite(*this);
        }

        [[nodiscard]]
        BlockReference<true> otherBlock()
            requires (isMut_)
        {
            return doOtherBlock(*this);
        }

        [[nodiscard]]
        BlockReference<false> otherBlock() const {
            return doOtherBlock(*this);
        }

        [[nodiscard]]
        SolverContactIndex solverIndex() const {
            if (!isValid()) {
                throw invalidError();
            }
            switch (index_.direction().id()) {
            case Direction::Id::plusX:
                return SolverContactIndex{ localBlock_->linkIndices().plusX, true};
            case Direction::Id::minusX:
                return SolverContactIndex{ otherBlock_->linkIndices().plusX, false};
            case Direction::Id::plusY:
                return SolverContactIndex{ localBlock_->linkIndices().plusY, true};
            case Direction::Id::minusY:
                return SolverContactIndex{ otherBlock_->linkIndices().plusY, false};
            case Direction::Id::plusZ:
                return SolverContactIndex{ localBlock_->linkIndices().plusZ, true};
            case Direction::Id::minusZ:
                return SolverContactIndex{ otherBlock_->linkIndices().plusZ, false};
            }
            throw index_.direction().invalidError();
        }

        [[nodiscard]]
        StructureReference<true> structure() &
            requires (isMut_)
        {
            return doStructure(*this);
        }

        [[nodiscard]]
        StructureReference<isMut_> structure() && {
            return doStructure(std::move(*this));
        }

        [[nodiscard]]
        StructureReference<false> structure() const& {
            return doStructure(*this);
        }

        [[nodiscard]]
        Real<u.length> thickness() const {
            if (!isValid()) {
                throw invalidError();
            }
            return structure_->sceneData().thicknessAlong(index_.direction());
        }

        template<bool mut>
        [[nodiscard]]
        bool operator==(ContactReference<libCfg, UD_, mut> const& rhs) const {
            return (structure_ == rhs.structure_) && (index_ == rhs.index_);
        }
    private:
        [[nodiscard]]
        static auto doCommonUserData(meta::cCvRefOf<ContactReference> auto&& self) -> decltype(self.commonUserData()) {
            if (!self.isValid()) {
                throw self.invalidError();
            }
            return self.structure_->sceneData().userData();
        }

        [[nodiscard]]
        static auto doLocalBlock(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.localBlock());
            if (!self.isValid()) {
                throw self.invalidError();
            }
            return Result{ self.structure_->sceneData(), self.index_.localBlockIndex() };
        }

        [[nodiscard]]
        static auto doOpposite(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.opposite());
            if (!self.isValid()) {
                throw self.invalidError();
            }
            return Result{ self.structure_->sceneData(), *self.index_.opposite() };
        }

        [[nodiscard]]
        static auto doOtherBlock(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.otherBlock());
            if (!self.isValid()) {
                throw self.invalidError();
            }
            return Result{ self.structure_->sceneData(), self.otherBlock_->index() };
        }

        [[nodiscard]]
        static auto doStructure(meta::cCvRefOf<ContactReference> auto&& self) {
            using Self = decltype(self);
            using Result = decltype(std::forward<Self>(self).structure());
            if (!self.isValid()) {
                throw self.invalidError();
            }
            return Result{ std::forward<Self>(self).structure_ };
        }

        PropSharedPtr<StructureData> structure_;
        PropPtr<BlockData> localBlock_;
        PropPtr<BlockData> otherBlock_;
        ContactIndex index_;
    };
}
