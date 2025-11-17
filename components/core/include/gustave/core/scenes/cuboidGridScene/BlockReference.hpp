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
#include <cstddef>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/blockReference/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/blockReference/Structures.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/meta/Meta.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_, bool isMut_>
    class BlockReference {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using BlockDataReference = detail::BlockDataReference<libCfg, false>;
        using DataNeighbours = detail::DataNeighbours<libCfg, false>;
        using IndexNeighbour = detail::IndexNeighbour;
        using IndexNeighbours = detail::IndexNeighbours;
        using SceneData = detail::SceneData<libCfg, UserData_>;
        using StructureData = SceneData::StructureData;
        using StructureIndex = StructureData::StructureIndex;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;
    public:
        using BlockIndex = cuboidGridScene::BlockIndex;
        using Direction = math3d::BasicDirection;
        using PressureStress = model::PressureStress<libCfg>;

        template<bool mut>
        using Contacts = blockReference::Contacts<libCfg, UserData_, mut>;

        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<libCfg, UserData_, mut>;

        template<bool mut>
        using Structures = blockReference::Structures<libCfg, UserData_, mut>;

        template<bool mut>
        using StructureReference = cuboidGridScene::StructureReference<libCfg, UserData_, mut>;

        [[nodiscard]]
        explicit BlockReference(Prop<SceneData>& sceneData, BlockIndex const& index)
            : sceneData_{ &sceneData }
            , index_{ index }
        {}

        [[nodiscard]]
        explicit BlockReference(utils::NoInit NO_INIT)
            : index_{ NO_INIT }
        {}

        [[nodiscard]]
        BlockReference(BlockReference&)
            requires (isMut_)
        = default;

        [[nodiscard]]
        BlockReference(BlockReference const&)
            requires (not isMut_)
        = default;

        [[nodiscard]]
        BlockReference(meta::cCvRefOf<BlockReference<libCfg, UserData_, true>> auto&& other)
            requires (not isMut_)
            : BlockReference{ std::forward<decltype(other)>(other).asImmutable()}
        {}

        [[nodiscard]]
        BlockReference(BlockReference&&) = default;

        BlockReference& operator=(BlockReference&)
            requires (isMut_)
        = default;

        BlockReference& operator=(BlockReference const&)
            requires (not isMut_)
        = default;

        BlockReference& operator=(meta::cCvRefOf<BlockReference<libCfg, UserData_, true>> auto&& other)
            requires (not isMut_)
        {
            *this = std::forward<decltype(other)>(other).asImmutable();
            return *this;
        }

        BlockReference& operator=(BlockReference&&) = default;

        [[nodiscard]]
        BlockReference<libCfg, UserData_, false> asImmutable() const {
            return BlockReference<libCfg, UserData_, false>{ *sceneData_, index_ };
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return sceneData_->blocks.blockSize();
        }

        [[nodiscard]]
        Contacts<true> contacts()
            requires (isMut_)
        {
            return Contacts<true>{ *sceneData_, index_ };
        }

        [[nodiscard]]
        Contacts<false> contacts() const {
            return Contacts<false>{ *sceneData_, index_ };
        }

        [[nodiscard]]
        BlockIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return data().isFoundation();
        }

        [[nodiscard]]
        bool isValid() const {
            return sceneData_->blocks.contains(index_);
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return data().mass();
        }

        [[nodiscard]]
        PressureStress const& maxPressureStress() const {
            return data().maxPressureStress();
        }

        [[nodiscard]]
        Vector3<u.length> position() const {
            using Rep = typename Real<u.length>::Rep;
            Vector3<u.length> const& bSize = blockSize();
            Real<u.length> const x = Rep(index_.x) * bSize.x();
            Real<u.length> const y = Rep(index_.y) * bSize.y();
            Real<u.length> const z = Rep(index_.z) * bSize.z();
            return Vector3<u.length>{ x, y, z };
        }

        [[nodiscard]]
        Structures<true> structures()
            requires (isMut_)
        {
            return Structures<true>{ *sceneData_, data() };
        }

        [[nodiscard]]
        Structures<false> structures() const {
            return Structures<false>{ *sceneData_, data() };
        }

        [[nodiscard]]
        bool operator==(BlockReference const&) const = default;
    private:
        PropPtr<SceneData> sceneData_;
        BlockIndex index_;

        [[nodiscard]]
        BlockDataReference data() const {
            BlockDataReference result = sceneData_->blocks.find(index_);
            if (!result) {
                std::stringstream msg;
                msg << "No block at index " << index_ << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }
    };
}
