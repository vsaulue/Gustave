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

#include <memory>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/Blocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneUpdater.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/Structures.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/scenes/cuboidGrid/TransactionResult.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class Scene {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using SceneData = detail::SceneData<cfg>;
        using SceneUpdater = detail::SceneUpdater<cfg>;
        using StructureData = detail::StructureData<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using BlockIndex = BlockPosition;
        using BlockReference = CuboidGrid::BlockReference<cfg>;
        using Blocks = CuboidGrid::Blocks<cfg>;
        using Direction = Math3d::BasicDirection;
        using StructureReference = CuboidGrid::StructureReference<cfg>;
        using Structures = CuboidGrid::Structures<cfg>;
        using Transaction = CuboidGrid::Transaction<cfg>;
        using TransactionResult = CuboidGrid::TransactionResult<cfg>;

        [[nodiscard]]
        explicit Scene(Vector3<u.length> const& blockSize)
            : data_{ blockSize }
        {}

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        TransactionResult modify(Transaction const& transaction) {
            auto upResult = SceneUpdater{ data_ }.runTransaction(transaction);
            return TransactionResult{ asReferences(std::move(upResult.newStructures)), asReferences(std::move(upResult.removedStructures)) };
        }

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ data_ };
        }

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const {
            return data_.blocks.blockSize();
        }

        [[nodiscard]]
        Real<u.area> contactAreaAlong(Direction direction) const {
            return data_.blocks.contactAreaAlong(direction);
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ data_ };
        }

        [[nodiscard]]
        Real<u.length> thicknessAlong(Direction direction) const {
            return data_.blocks.thicknessAlong(direction);
        }
    private:
        [[nodiscard]]
        std::vector<StructureReference> asReferences(std::vector<std::shared_ptr<StructureData const>>&& structures) {
            std::vector<StructureReference> result;
            result.reserve(structures.size());
            for (auto&& structurePtr : structures) {
                result.emplace_back(std::move(structurePtr));
            }
            return result;
        }

        SceneData data_;
    };
}
