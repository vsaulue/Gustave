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
#include <limits>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/common/UserDataTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/utils/IndexGenerator.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_>
    class BlockMappedData {
    private:
        static constexpr auto u = cfg::units(cfg);

        template<cfg::cUnitOf<cfg> auto unit>
        using Real = cfg::Real<cfg, unit>;
    public:
        using UDTraits = common::UserDataTraits<UD_>;

        using LinkIndex = cfg::LinkIndex<cfg>;
        using PressureStress = model::PressureStress<cfg>;
        using StructureIndex = cfg::StructureIndex<cfg>;
        using UserDataMember = UDTraits::BlockMember;

        struct LinkIndices {
            LinkIndex plusX;
            LinkIndex plusY;
            LinkIndex plusZ;
        };

        [[nodiscard]]
        static constexpr bool hasUserData() {
            return UDTraits::hasBlockUserData();
        }

        [[nodiscard]]
        explicit BlockMappedData(BlockConstructionInfo<cfg> const& info)
            : maxPressureStress_{ info.maxPressureStress() }
            , linkIndices_{ maxLinkId(), maxLinkId(), maxLinkId() }
            , mass_{ info.mass() }
            , isFoundation_{ info.isFoundation() }
            , structureId_{ utils::IndexGenerator<StructureIndex>::invalidIndex() }
        {
            assert(mass_ > 0.f * u.mass);
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
        bool isFoundation() const {
            return isFoundation_;
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
        StructureIndex& structureId() {
            return structureId_;
        }

        [[nodiscard]]
        StructureIndex structureId() const {
            return structureId_;
        }

        [[nodiscard]]
        UserDataMember const& userData() const
            requires (hasUserData())
        {
            return userData_;
        }

        [[nodiscard]]
        UserDataMember& userData()
            requires (hasUserData())
        {
            return userData_;
        }
    private:
        [[nodiscard]]
        static constexpr LinkIndex maxLinkId() {
            return std::numeric_limits<LinkIndex>::max();
        }

        PressureStress maxPressureStress_;
        LinkIndices linkIndices_;
        Real<u.mass> mass_;
        bool isFoundation_;

        [[no_unique_address]]
        UserDataMember userData_;

        StructureIndex structureId_;
    };
}
