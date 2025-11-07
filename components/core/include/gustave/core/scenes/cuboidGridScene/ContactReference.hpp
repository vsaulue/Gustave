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

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockDataReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/utils/Prop.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_, bool isMut_>
    class ContactReference {
    private:
        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        friend class ContactReference;

        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        static constexpr auto u = cfg::units(libCfg);

        using BlockDataReference = detail::BlockDataReference<libCfg, false>;
        using SceneData = detail::SceneData<libCfg, UserData_>;
        using StructureData = SceneData::StructureData;
        using StructureIndex = SceneData::StructureIndex;

        struct BlockDatas {
            BlockDataReference local;
            BlockDataReference other;

            [[nodiscard]]
            bool isValid() const {
                if (local && other) {
                    return !local.isFoundation() || !other.isFoundation();
                }
                return false;
            }

            [[nodiscard]]
            std::optional<StructureIndex> structureId() const {
                if (!isValid()) {
                    return {};
                }
                if (!local.isFoundation()) {
                    return local.structureId();
                } else {
                    return other.structureId();
                }
            }
        };

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using AsImmutable = ContactReference<libCfg, UserData_, false>;
        using BlockIndex = cuboidGridScene::BlockIndex;
        using ContactIndex = cuboidGridScene::ContactIndex;
        using Direction = math3d::BasicDirection;
        using PressureStress = model::PressureStress<libCfg>;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
        using SolverContactIndex = typename solvers::Structure<libCfg>::ContactIndex;

        template<bool mut>
        using BlockReference = cuboidGridScene::BlockReference<libCfg, UserData_, mut>;

        template<bool mut>
        using StructureReference = cuboidGridScene::StructureReference<libCfg, UserData_, mut>;

        [[nodiscard]]
        explicit ContactReference(utils::NoInit NO_INIT)
            : scene_{ nullptr }
            , index_{ NO_INIT }
        {}

        [[nodiscard]]
        explicit ContactReference(Prop<SceneData>& scene, ContactIndex const& index)
            : scene_{ &scene }
            , index_{ index }
        {}

        [[nodiscard]]
        ContactReference(ContactReference&)
            requires (isMut_)
        = default;

        [[nodiscard]]
        ContactReference(ContactReference const&)
            requires (not isMut_)
        = default;

        [[nodiscard]]
        ContactReference(meta::cCvRefOf<ContactReference<libCfg, UserData_, true>> auto&& other)
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

        ContactReference& operator=(meta::cCvRefOf<ContactReference<libCfg, UserData_, true>> auto&& rhs)
            requires (not isMut_)
        {
            *this = std::forward<decltype(rhs)>(rhs).asImmutable();
            return *this;
        }

        ContactReference& operator=(ContactReference&&) = default;

        [[nodiscard]]
        Real<u.area> area() const {
            return scene_->blocks.contactAreaAlong(index_.direction());
        }

        [[nodiscard]]
        AsImmutable asImmutable() const
            requires (isMut_)
        {
            return AsImmutable{ scene_, index_ };
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
            return blockDatas().isValid();
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
            BlockDatas blocks = blockDatas();
            if (!blocks.isValid()) {
                throw invalidError();
            }
            return PressureStress::minStress(blocks.local.maxPressureStress(), blocks.other.maxPressureStress());
        }

        [[nodiscard]]
        NormalizedVector3 normal() const {
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
            BlockDatas datas = blockDatas();
            if (!datas.isValid()) {
                throw invalidError();
            }
            switch (index_.direction().id()) {
            case Direction::Id::plusX:
                return SolverContactIndex{ datas.local.linkIndices().plusX, true};
            case Direction::Id::minusX:
                return SolverContactIndex{ datas.other.linkIndices().plusX, false};
            case Direction::Id::plusY:
                return SolverContactIndex{ datas.local.linkIndices().plusY, true};
            case Direction::Id::minusY:
                return SolverContactIndex{ datas.other.linkIndices().plusY, false};
            case Direction::Id::plusZ:
                return SolverContactIndex{ datas.local.linkIndices().plusZ, true};
            case Direction::Id::minusZ:
                return SolverContactIndex{ datas.other.linkIndices().plusZ, false};
            }
            throw std::invalid_argument(index_.direction().invalidValueMsg());
        }

        [[nodiscard]]
        StructureReference<true> structure()
            requires (isMut_)
        {
            return doStructure(*this);
        }

        [[nodiscard]]
        StructureReference<false> structure() const {
            return doStructure(*this);
        }

        [[nodiscard]]
        Real<u.length> thickness() const {
            return scene_->blocks.thicknessAlong(index_.direction());
        }

        template<bool mut>
        [[nodiscard]]
        bool operator==(ContactReference<libCfg, UserData_, mut> const& rhs) const {
            return (scene_ == rhs.scene_) && (index_ == rhs.index_);
        }
    private:
        [[nodiscard]]
        BlockDatas blockDatas() const {
            BlockDataReference local = scene_->blocks.find(index_.localBlockIndex());
            BlockDataReference other{ nullptr };
            if (auto optOtherIndex = index_.otherBlockIndex()) {
                other = scene_->blocks.find(*optOtherIndex);
            }
            return BlockDatas{ local, other };
        }

        [[nodiscard]]
        static auto doLocalBlock(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.localBlock());
            auto result = Result{ *self.scene_, self.index_.localBlockIndex() };
            if (!result.isValid()) {
                throw self.invalidError();
            }
            return result;
        }

        [[nodiscard]]
        static auto doOpposite(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.opposite());
            BlockDatas blocks = self.blockDatas();
            if (!blocks.isValid()) {
                throw self.invalidError();
            }
            return Result{ *self.scene_, { blocks.other.index(), self.index_.direction().opposite()} };
        }

        [[nodiscard]]
        static auto doOtherBlock(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.otherBlock());
            std::optional<BlockIndex> blockId = self.index_.otherBlockIndex();
            if (!blockId || !self.scene_->blocks.contains(*blockId)) {
                throw self.invalidError();
            }
            return Result{ *self.scene_, *blockId };
        }

        [[nodiscard]]
        static auto doStructure(meta::cCvRefOf<ContactReference> auto&& self) {
            using Result = decltype(self.structure());
            auto const datas = self.blockDatas();
            auto const optStructId = datas.structureId();
            if (!optStructId) {
                throw self.invalidError();
            }
            return Result{ self.scene_->structures.atShared(*optStructId) };
        }

        PropPtr<SceneData> scene_;
        ContactIndex index_;
    };
}
