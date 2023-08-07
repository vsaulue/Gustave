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

#include <functional>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/meta/MutableIf.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockData.hpp>
#include <gustave/utils/HashEquals.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg, bool isMutable_>
    class BlockReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using QualifiedBlockData = Meta::MutableIf<isMutable_, BlockData<cfg>>;

        using Material = Model::Material<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        QualifiedBlockData* data_;
    public:
        using Hasher = Utils::Hasher<BlockReference, &BlockReference::data_>;

        [[nodiscard]]
        static constexpr bool isMutable() {
            return isMutable_;
        }

        [[nodiscard]]
        BlockReference(QualifiedBlockData* data)
            : data_{ data }
        {}

        [[nodiscard]]
        explicit BlockReference(Utils::NoInit) {}

        [[nodiscard]]
        BlockReference(BlockReference const&) = default;

        BlockReference& operator=(BlockReference const&) = default;

        [[nodiscard]]
        BlockReference(BlockReference<cfg, true> const& otherBlock)
            requires (!isMutable_)
            : data_{ otherBlock.data() }
        {}

        BlockReference& operator=(BlockReference<cfg, true> const& rhs)
            requires (!isMutable_)
        {
            data_ = rhs.data();
            return *this;
        }

        template<bool rhsMutable>
        bool operator==(BlockReference<cfg, rhsMutable> const& rhs) const {
            return data_ == rhs.data();
        }

        [[nodiscard]]
        BlockData<cfg> const* data() const {
            return data_;
        }

        [[nodiscard]]
        SceneStructure<cfg>*& structure()
            requires (isMutable_)
        {
            return data_->second.structure();
        }

        [[nodiscard]]
        SceneStructure<cfg> const* structure() const {
            return data_->second.structure();
        }

        [[nodiscard]]
        BlockPosition const& position() const {
            return data_->first;
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return data_->second.mass();
        }

        [[nodiscard]]
        Material const& material() const {
            return data_->second.material();
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

template<Gustave::Cfg::cLibConfig auto cfg, bool isMutable>
struct std::hash<Gustave::Scenes::CuboidGrid::detail::BlockReference<cfg,isMutable>> : Gustave::Scenes::CuboidGrid::detail::BlockReference<cfg,isMutable>::Hasher {};
