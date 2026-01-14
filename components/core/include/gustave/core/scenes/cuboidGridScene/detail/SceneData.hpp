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

#include <memory>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/SharedIndexedSet.hpp>
#include <gustave/utils/IndexGenerator.hpp>
#include <gustave/utils/PointerHash.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_>
    struct SceneData {
    public:
        using UserData = UD_;

        using BlockData = detail::BlockData<cfg, UD_>;
        using BlockIndex = cuboidGridScene::BlockIndex;
        using Direction = math3d::BasicDirection;
        using StructureData = detail::StructureData<cfg, UD_>;
        using StructureIndex = cfg::StructureIndex<cfg>;
    private:
        static constexpr auto u = cfg::units(cfg);

        template<cfg::cUnitOf<cfg> auto unit>
        using Vector3 = cfg::Vector3<cfg, unit>;

        template<cfg::cUnitOf<cfg> auto unit>
        using Real = cfg::Real<cfg, unit>;
    public:
        using Blocks = utils::SharedIndexedSet<BlockData>;
        using StructureIdGenerator = utils::IndexGenerator<StructureIndex>;
        using Structures = utils::SharedIndexedSet<StructureData>;

        [[nodiscard]]
        explicit SceneData(Vector3<u.length> const& blockSize)
            : blockSize_{ blockSize }
        {
            if (blockSize.x() <= 0.f * u.length) {
                throw blockSizeError('x', blockSize.x());
            }
            if (blockSize.y() <= 0.f * u.length) {
                throw blockSizeError('y', blockSize.y());
            }
            if (blockSize.z() <= 0.f * u.length) {
                throw blockSizeError('z', blockSize.z());
            }
        }

        SceneData(SceneData const&) = delete;
        SceneData& operator=(SceneData const&) = delete;

        [[nodiscard]]
        SceneData(SceneData&& other)
            : blocks{ std::move(other.blocks) }
            , structures{ std::move(other.structures) }
            , structureIdGenerator{ other.structureIdGenerator }
            , blockSize_{ std::move(other.blockSize_) }
        {
            resetSceneDataPtr();
        }

        SceneData& operator=(SceneData&& other) {
            if (&other != this) {
                blocks = std::move(other.blocks);
                structures = std::move(other.structures);
                structureIdGenerator = other.structureIdGenerator;
                blockSize_ = other.blockSize_;
                resetSceneDataPtr();
            }
            return *this;
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return blockSize_;
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            auto const& dims = blockSize_;
            switch (direction.id()) {
            case Direction::Id::plusX:
            case Direction::Id::minusX:
                return dims.y() * dims.z();
            case Direction::Id::plusY:
            case Direction::Id::minusY:
                return dims.x() * dims.z();
            case Direction::Id::plusZ:
            case Direction::Id::minusZ:
                return dims.x() * dims.y();
            }
            throw direction.invalidError();
        }

        [[nodiscard]]
        bool isStructureValid(StructureData const* structure) const {
            return structure != nullptr && structures.contains(structure);
        }

        [[nodiscard]]
        bool isStructureIdValid(StructureIndex id) const {
            return id != structureIdGenerator.invalidIndex() && structures.contains(id);
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            switch (direction.id()) {
            case Direction::Id::plusX:
            case Direction::Id::minusX:
                return blockSize_.x();
            case Direction::Id::plusY:
            case Direction::Id::minusY:
                return blockSize_.y();
            case Direction::Id::plusZ:
            case Direction::Id::minusZ:
                return blockSize_.z();
            }
            throw direction.invalidError();
        }

        Blocks blocks;
        Structures structures;
        StructureIdGenerator structureIdGenerator;
    private:
        [[nodiscard]]
        static std::invalid_argument blockSizeError(char coordSymbol, Real<u.length> const value) {
            std::stringstream result;
            result << "blocksize." << coordSymbol << " must be strictly positive (passed: " << value << ").";
            return std::invalid_argument{ result.str() };
        }

        void resetSceneDataPtr() {
            for (auto& structure : structures) {
                structure->setSceneData(*this);
            }
        }

        Vector3<u.length> blockSize_;
    };
}
