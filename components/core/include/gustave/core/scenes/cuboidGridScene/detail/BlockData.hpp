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

#include <cassert>
#include <limits>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/common/UserDataTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/IndexGenerator.hpp>
#include <gustave/utils/prop/Ptr.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_>
    class BlockData {
        static constexpr auto u = cfg::units(libCfg);

        using NodeIndex = cfg::NodeIndex<libCfg>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using UDTraits = common::UserDataTraits<UD_>;
    public:
        using BlockConstructionInfo = cuboidGridScene::BlockConstructionInfo<libCfg>;
        using BlockIndex = cuboidGridScene::BlockIndex;
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using PressureStress = model::PressureStress<libCfg>;
        using SceneData = detail::SceneData<libCfg, UD_>;
        using StructureIndex = cfg::StructureIndex<libCfg>;
        using UserDataMember = UDTraits::BlockMember;

        struct LinkIndices {
            LinkIndex plusX;
            LinkIndex plusY;
            LinkIndex plusZ;
        };

        [[nodiscard]]
        explicit BlockData(BlockConstructionInfo const& info, SceneData& scene)
            : index_{ info.index() }
            , maxPressureStress_{ info.maxPressureStress() }
            , linkIndices_{ maxLinkId(), maxLinkId(), maxLinkId() }
            , mass_{ info.mass() }
            , isFoundation_{ info.isFoundation() }
            , isValid_{ true }
            , structureId_{ utils::IndexGenerator<StructureIndex>::invalidIndex() }
            , scene_{ &scene }
        {
            assert(mass_ > 0.f * u.mass);
        }

        [[nodiscard]]
        LinkIndices& linkIndices() {
            return linkIndices_;
        }

        [[nodiscard]]
        LinkIndices const& linkIndices() const {
            return linkIndices_;
        }

        [[nodiscard]]
        BlockIndex const& index() const {
            return index_;
        }

        void invalidate() {
            assert(isValid_);
            isValid_ = false;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return isFoundation_;
        }

        [[nodiscard]]
        bool isValid() const {
            return isValid_;
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return mass_;
        }

        [[nodiscard]]
        PressureStress const& maxPressureStress() const {
            return maxPressureStress_;
        }

        [[nodiscard]]
        SceneData& sceneData() {
            return *scene_;
        }

        [[nodiscard]]
        SceneData const& sceneData() const {
            return *scene_;
        }

        void setSceneData(SceneData& value) {
            scene_ = &value;
        }

        [[nodiscard]]
        StructureIndex& structureId() {
            return structureId_;
        }

        [[nodiscard]]
        StructureIndex structureId() const {
            return structureId_;
        }

        [[nodiscard]]
        UserDataMember& userData()
            requires (UDTraits::hasBlockUserData())
        {
            return userData_;
        }

        [[nodiscard]]
        UserDataMember const& userData() const
            requires (UDTraits::hasBlockUserData())
        {
            return userData_;
        }
    private:
        [[nodiscard]]
        static constexpr LinkIndex maxLinkId() {
            return std::numeric_limits<LinkIndex>::max();
        }

        BlockIndex index_;
        PressureStress maxPressureStress_;
        LinkIndices linkIndices_;
        Real<u.mass> mass_;
        bool isFoundation_;
        bool isValid_;

        [[no_unique_address]]
        UserDataMember userData_;

        StructureIndex structureId_;
        utils::prop::Ptr<SceneData> scene_;
    };
}
