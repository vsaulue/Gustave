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
#include <optional>
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldUpdater.hpp>
#include <gustave/core/worlds/syncWorld/Blocks.hpp>
#include <gustave/core/worlds/syncWorld/Contacts.hpp>
#include <gustave/core/worlds/syncWorld/Links.hpp>
#include <gustave/core/worlds/syncWorld/Structures.hpp>

namespace gustave::core::worlds {
    template<cfg::cLibConfig auto libCfg>
    class SyncWorld {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using WorldData = syncWorld::detail::WorldData<libCfg>;
        using WorldUpdater = syncWorld::detail::WorldUpdater<libCfg>;
    public:
        using Blocks = syncWorld::Blocks<libCfg>;
        using Contacts = syncWorld::Contacts<libCfg>;
        using Links = syncWorld::Links<libCfg>;
        using Structures = syncWorld::Structures<libCfg>;

        using BlockIndex = WorldData::Scene::BlockIndex;
        using BlockReference = Blocks::BlockReference;
        using ContactIndex = Contacts::ContactIndex;
        using ContactReference = Contacts::ContactReference;
        using Scene = WorldData::Scene;
        using Solver = WorldData::Solver;
        using StructureReference = Structures::StructureReference;
        using Transaction = WorldData::Scene::Transaction;
        using TransactionResult = WorldData::Scene::TransactionResult;

        [[nodiscard]]
        explicit SyncWorld(Vector3<u.length> const& blockSize, Solver solver)
            : data_{ blockSize, std::move(solver) }
        {}

        SyncWorld(SyncWorld const&) = delete;
        SyncWorld& operator=(SyncWorld const&) = delete;

        [[nodiscard]]
        SyncWorld(SyncWorld&&) = default;
        SyncWorld& operator=(SyncWorld&&) = default;

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ data_ };
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ data_ };
        }

        [[nodiscard]]
        Vector3<u.acceleration> g() const {
            return data_.solver.config().g();
        }

        [[nodiscard]]
        Links links() const {
            return Links{ data_ };
        }

        TransactionResult modify(Transaction const& transaction) {
            return WorldUpdater{ data_ }.runTransaction(transaction);
        }

        [[nodiscard]]
        Scene const& scene() const {
            return data_.scene;
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ data_ };
        }
    private:
        WorldData data_;
    };
}
