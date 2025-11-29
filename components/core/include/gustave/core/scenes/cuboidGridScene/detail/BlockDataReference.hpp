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

#include <functional>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/utils/HashEquals.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_, bool isMut_>
    class BlockDataReference {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        using NodeIndex = cfg::NodeIndex<libCfg>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using Data = BlockData<libCfg, UD_>;

        Prop<Data>* data_;
    public:
        using Hasher = utils::Hasher<BlockDataReference, &BlockDataReference::data_>;
        using LinkIndices = BlockMappedData<libCfg, UD_>::LinkIndices;
        using PressureStress = model::PressureStress<libCfg>;
        using StructureIndex = BlockMappedData<libCfg, UD_>::StructureIndex;

        [[nodiscard]]
        static constexpr bool isMutable() {
            return isMut_;
        }

        [[nodiscard]]
        BlockDataReference(Prop<BlockData<libCfg, UD_>>* data)
            : data_{ data }
        {}

        [[nodiscard]]
        explicit BlockDataReference(utils::NoInit) {}

        [[nodiscard]]
        BlockDataReference(BlockDataReference const&) = default;

        BlockDataReference& operator=(BlockDataReference const&) = default;

        [[nodiscard]]
        BlockDataReference(BlockDataReference<libCfg, UD_, true> const& otherBlock)
            requires (!isMut_)
            : data_{ otherBlock.data() }
        {}

        BlockDataReference& operator=(BlockDataReference<libCfg, UD_, true> const& rhs)
            requires (!isMut_)
        {
            data_ = rhs.data();
            return *this;
        }

        template<bool rhsMutable>
        [[nodiscard]]
        bool operator==(BlockDataReference<libCfg, UD_, rhsMutable> const& rhs) const {
            return data_ == rhs.data();
        }

        [[nodiscard]]
        BlockData<libCfg, UD_> const* data() const {
            return data_;
        }

        [[nodiscard]]
        Prop<LinkIndices>& linkIndices() const {
            return data_->second.linkIndices();
        }

        [[nodiscard]]
        StructureIndex& structureId()
            requires (isMut_)
        {
            return data_->second.structureId();
        }

        [[nodiscard]]
        StructureIndex structureId() const {
            return data_->second.structureId();
        }

        [[nodiscard]]
        BlockIndex const& index() const {
            return data_->first;
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return data_->second.mass();
        }

        [[nodiscard]]
        PressureStress const& maxPressureStress() const {
            return data_->second.maxPressureStress();
        }

        [[nodiscard]]
        bool isFoundation() const {
            return data_->second.isFoundation();
        }

        [[nodiscard]]
        operator bool() const {
            return data_ != nullptr;
        }
    };
}

template<gustave::cfg::cLibConfig auto libCfg, gustave::core::scenes::common::cSceneUserData UD, bool isMut>
struct std::hash<gustave::core::scenes::cuboidGridScene::detail::BlockDataReference<libCfg,UD,isMut>>
    : public gustave::core::scenes::cuboidGridScene::detail::BlockDataReference<libCfg,UD,isMut>::Hasher
{};
