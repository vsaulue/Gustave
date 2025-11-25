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

#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockDataReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockMappedData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto libCfg>
    class SceneBlocks {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;
    public:
        using BlockConstructionInfo = cuboidGridScene::BlockConstructionInfo<libCfg>;
        using BlockIndex = cuboidGridScene::BlockIndex;
        using BlockMap = std::unordered_map<BlockIndex, BlockMappedData<libCfg>>;
        using Direction = math3d::BasicDirection;

        using const_iterator = BlockMap::const_iterator;

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
        BlockDataReference<libCfg, true> at(BlockIndex const& index) {
            return doAt(*this, index);
        }

        [[nodiscard]]
        BlockDataReference<libCfg, false> at(BlockIndex const& index) const {
            return doAt(*this, index);
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return blockSize_;
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            Vector3<u.length> const& dims = blockSize_;
            Real<u.area> result = 0.f * u.area;
            switch (direction.id()) {
            case Direction::Id::plusX:
            case Direction::Id::minusX:
                result = dims.y() * dims.z();
                break;
            case Direction::Id::plusY:
            case Direction::Id::minusY:
                result = dims.x() * dims.z();
                break;
            case Direction::Id::plusZ:
            case Direction::Id::minusZ:
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
        BlockDataReference<libCfg,true> find(BlockIndex const& index) {
            return doFind(*this, index);
        }

        [[nodiscard]]
        BlockDataReference<libCfg,false> find(BlockIndex const& index) const {
            return doFind(*this, index);
        }

        BlockDataReference<libCfg, true> insert(BlockConstructionInfo const& info) {
            auto it = blocks_.emplace(info.index(), BlockMappedData{ info }).first;
            return { &(*it) };
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            Real<u.length> result = 0.f * u.length;
            switch (direction.id()) {
            case Direction::Id::plusX:
            case Direction::Id::minusX:
                result = blockSize_.x();
                break;
            case Direction::Id::plusY:
            case Direction::Id::minusY:
                result = blockSize_.y();
                break;
            case Direction::Id::plusZ:
            case Direction::Id::minusZ:
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
        static auto doAt(meta::cCvRefOf<SceneBlocks> auto&& self, BlockIndex const& index) -> decltype(self.at(index)) {
            auto it = self.blocks_.find(index);
            if (it == self.blocks_.end()) {
                throw invalidIndexError(index);
            }
            return { &(*it) };
        }

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

        [[nodiscard]]
        static std::out_of_range invalidIndexError(BlockIndex const& blockIndex) {
            std::stringstream msg;
            msg << "Block at index " << blockIndex << " does not exists.";
            return std::out_of_range(msg.str());
        }

        Vector3<u.length> blockSize_;
        BlockMap blocks_;
    };
}
