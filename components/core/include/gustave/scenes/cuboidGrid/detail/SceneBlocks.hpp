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

#include <unordered_map>
#include <string>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockMappedData.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg>
    class SceneBlocks {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using BlockMap = std::unordered_map<BlockPosition, BlockMappedData<cfg>>;
        using Direction = Math3d::BasicDirection;

        [[nodiscard]]
        explicit SceneBlocks(Vector3<u.length> const& blockSize)
            : blockSize_{ blockSize }
        {
            if (blockSize.x() <= 0.f * u.length) {
                throw std::invalid_argument{ blockSizeError('x', blockSize.x()) };
            }
            if (blockSize.y() <= 0.f * u.length) {
                throw std::invalid_argument{ blockSizeError('y', blockSize.y()) };
            }
            if (blockSize.z() <= 0.f * u.length) {
                throw std::invalid_argument{ blockSizeError('z', blockSize.z()) };
            }
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return blockSize_;
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            Vector3<u.length> const& dims = blockSize_;
            Real<u.area> result = 0.f * u.area;
            switch (direction) {
            case Direction::plusX:
            case Direction::minusX:
                result = dims.y() * dims.z();
                break;
            case Direction::plusY:
            case Direction::minusY:
                result = dims.x() * dims.z();
                break;
            case Direction::plusZ:
            case Direction::minusZ:
                result = dims.x() * dims.y();
                break;
            }
            return result;
        }

        [[nodiscard]]
        bool contains(BlockPosition const& position) const {
            return blocks_.contains(position);
        }

        bool erase(BlockPosition const& position) {
            auto res = blocks_.erase(position);
            return res != 0;
        }

        [[nodiscard]]
        BlockDataReference<cfg,true> find(BlockPosition const& position) {
            return doFind(*this, position);
        }

        [[nodiscard]]
        BlockDataReference<cfg,false> find(BlockPosition const& position) const {
            return doFind(*this, position);
        }

        BlockDataReference<cfg, true> insert(BlockConstructionInfo<cfg> const& info) {
            auto it = blocks_.emplace(info.position(), BlockMappedData{ info }).first;
            return { &(*it) };
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            Real<u.length> result = 0.f * u.length;
            switch (direction) {
            case Direction::plusX:
            case Direction::minusX:
                result = blockSize_.x();
                break;
            case Direction::plusY:
            case Direction::minusY:
                result = blockSize_.y();
                break;
            case Direction::plusZ:
            case Direction::minusZ:
                result = blockSize_.z();
                break;
            }
            return result;
        }

        [[nodiscard]]
        std::size_t size() const {
            return blocks_.size();
        }
    private:
        [[nodiscard]]
        static auto doFind(auto&& self, BlockPosition const& position) -> decltype(self.find(position)) {
            auto it = self.blocks_.find(position);
            if (it != self.blocks_.end()) {
                return { &(*it) };
            } else {
                return { nullptr };
            }
        }

        [[nodiscard]]
        static std::string blockSizeError(char coordSymbol, Real<u.length> const value) {
            std::stringstream result;
            result << "blocksize." << coordSymbol << " must be strictly positive (passed: " << value << ").";
            return result.str();
        }

        Vector3<u.length> blockSize_;
        BlockMap blocks_;
    };
}
