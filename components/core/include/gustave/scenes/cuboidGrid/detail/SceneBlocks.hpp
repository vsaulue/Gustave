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
#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
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
        using BlockMap = std::unordered_map<BlockIndex, BlockMappedData<cfg>>;
        using Direction = Math3d::BasicDirection;

        using const_iterator = typename BlockMap::const_iterator;

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
        bool contains(BlockIndex const& index) const {
            return blocks_.contains(index);
        }

        bool erase(BlockIndex const& index) {
            auto res = blocks_.erase(index);
            return res != 0;
        }

        [[nodiscard]]
        const_iterator begin() const {
            return blocks_.begin();
        }

        [[nodiscard]]
        const_iterator end() const {
            return blocks_.end();
        }

        [[nodiscard]]
        BlockDataReference<cfg,true> find(BlockIndex const& index) {
            return doFind(*this, index);
        }

        [[nodiscard]]
        BlockDataReference<cfg,false> find(BlockIndex const& index) const {
            return doFind(*this, index);
        }

        BlockDataReference<cfg, true> insert(BlockConstructionInfo<cfg> const& info) {
            auto it = blocks_.emplace(info.index(), BlockMappedData{ info }).first;
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
        static auto doFind(auto&& self, BlockIndex const& index) -> decltype(self.find(index)) {
            auto it = self.blocks_.find(index);
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
