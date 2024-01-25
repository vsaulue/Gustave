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

#include <cassert>
#include <optional>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/scenes/CuboidGridScene.hpp>
#include <gustave/solvers/Force1Solver.hpp>
#include <gustave/utils/PointerHash.hpp>
#include <gustave/worlds/sync/BlockReference.hpp>
#include <gustave/worlds/sync/Blocks.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>
#include <gustave/worlds/sync/detail/WorldUpdater.hpp>
#include <gustave/worlds/sync/StructureReference.hpp>
#include <gustave/worlds/sync/Structures.hpp>
#include <gustave/worlds/WorldStructureState.hpp>

namespace Gustave::Worlds {
    template<Cfg::cLibConfig auto cfg>
    class SyncWorld {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;

        using WorldData = Sync::detail::WorldData<cfg>;
        using WorldUpdater = Sync::detail::WorldUpdater<cfg>;
    public:
        using Solver = Solvers::Force1Solver<cfg>;

        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using BlockReference = Sync::BlockReference<cfg>;
        using Blocks = Sync::Blocks<cfg>;
        using StructureReference = Sync::StructureReference<cfg>;
        using Structures = Sync::Structures<cfg>;
        using Transaction = typename WorldData::Scene::Transaction;

        [[nodiscard]]
        explicit SyncWorld(Vector3<u.length> const& blockSize, Solver solver)
            : data_{ blockSize, std::move(solver) }
        {}

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ data_ };
        }

        void modify(Transaction const& transaction) {
            WorldUpdater{ data_ }.runTransaction(transaction);
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ data_ };
        }
    private:
        WorldData data_;
    };
}
