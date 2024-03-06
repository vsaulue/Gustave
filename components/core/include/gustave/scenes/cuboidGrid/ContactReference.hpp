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

#include <sstream>
#include <stdexcept>
#include <string>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/model/MaxStress.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/ContactIndex.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class BlockReference;

    template<Cfg::cLibConfig auto cfg>
    class StructureReference;

    template<Cfg::cLibConfig auto cfg>
    class ContactReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using BlockDataReference = detail::BlockDataReference<cfg, false>;
        using SceneData = detail::SceneData<cfg>;
        using StructureData = detail::StructureData<cfg>;

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
        };

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;
    public:
        using BlockIndex = CuboidGrid::BlockIndex;
        using BlockReference = CuboidGrid::BlockReference<cfg>;
        using ContactIndex = CuboidGrid::ContactIndex;
        using Direction = Math3d::BasicDirection;
        using MaxStress = Model::MaxStress<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SolverContactIndex = typename Solvers::Structure<cfg>::ContactIndex;
        using StructureReference = CuboidGrid::StructureReference<cfg>;

        [[nodiscard]]
        explicit ContactReference(Utils::NoInit)
            : scene_{ nullptr }
            , index_{ ContactIndex{ {0,0,0}, Direction::plusX() } }
        {}

        [[nodiscard]]
        explicit ContactReference(SceneData const& scene, ContactIndex const& index)
            : scene_{ &scene }
            , index_{ index }
        {}

        [[nodiscard]]
        Real<u.area> area() const {
            return scene_->blocks.contactAreaAlong(index_.direction());
        }

        [[nodiscard]]
        ContactIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        std::string invalidMessage() const {
            std::stringstream msg;
            msg << "Invalid contact at index " << index_ << '.';
            return msg.str();
        }

        [[nodiscard]]
        bool isValid() const {
            return blockDatas().isValid();
        }

        [[nodiscard]]
        BlockReference localBlock() const {
            BlockReference result{ *scene_, index_.localBlockIndex() };
            if (!result.isValid()) {
                throw std::out_of_range(invalidMessage());
            }
            return result;
        }

        [[nodiscard]]
        MaxStress maxStress() const {
            BlockDatas blocks = blockDatas();
            if (!blocks.isValid()) {
                throw std::out_of_range(invalidMessage());
            }
            return MaxStress::minResistance(blocks.local.maxStress(), blocks.other.maxStress());
        }

        [[nodiscard]]
        NormalizedVector3 normal() const {
            return NormalizedVector3::basisVector(index_.direction());
        }

        [[nodiscard]]
        ContactReference opposite() const {
            BlockDatas blocks = blockDatas();
            if (!blocks.isValid()) {
                throw std::out_of_range(invalidMessage());
            }
            return ContactReference{ *scene_, ContactIndex{ blocks.other.index(), index_.direction().opposite()}};
        }

        [[nodiscard]]
        BlockReference otherBlock() const {
            std::optional<BlockIndex> blockId = index_.otherBlockIndex();
            if (!blockId || !scene_->blocks.contains(*blockId)) {
                throw std::out_of_range(invalidMessage());
            }
            return BlockReference{ *scene_, *blockId };
        }

        [[nodiscard]]
        SolverContactIndex solverIndex() const {
            BlockDatas datas = blockDatas();
            if (!datas.isValid()) {
                throw std::out_of_range(invalidMessage());
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
        StructureReference structure() const {
            BlockDatas datas = blockDatas();
            if (!datas.isValid()) {
                throw std::out_of_range(invalidMessage());
            }
            StructureData const* localStruct = datas.local.structure();
            auto rawStruct = (localStruct != nullptr) ? localStruct : datas.other.structure();
            std::shared_ptr<StructureData const> sharedStruct = *scene_->structures.find(rawStruct);
            assert(sharedStruct);
            return StructureReference{ std::move(sharedStruct) };
        }

        [[nodiscard]]
        Real<u.length> thickness() const {
            return scene_->blocks.thicknessAlong(index_.direction());
        }

        [[nodiscard]]
        bool operator==(ContactReference const&) const = default;
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

        SceneData const* scene_;
        ContactIndex index_;
    };
}
