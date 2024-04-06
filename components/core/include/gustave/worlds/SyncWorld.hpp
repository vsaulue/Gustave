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
#include <gustave/worlds/sync/Blocks.hpp>
#include <gustave/worlds/sync/Contacts.hpp>
#include <gustave/worlds/sync/Links.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>
#include <gustave/worlds/sync/detail/WorldUpdater.hpp>
#include <gustave/worlds/sync/Structures.hpp>
#include <gustave/worlds/WorldStructureState.hpp>

namespace gustave::worlds {
    template<cfg::cLibConfig auto libCfg>
    class SyncWorld {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using WorldData = sync::detail::WorldData<libCfg>;
        using WorldUpdater = sync::detail::WorldUpdater<libCfg>;
    public:
        using Blocks = sync::Blocks<libCfg>;
        using Contacts = sync::Contacts<libCfg>;
        using Links = sync::Links<libCfg>;
        using Structures = sync::Structures<libCfg>;

        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using BlockReference = typename Blocks::BlockReference;
        using ContactIndex = typename Contacts::ContactIndex;
        using ContactReference = typename Contacts::ContactReference;
        using Solver = typename WorldData::Solver;
        using StructureReference = typename Structures::StructureReference;
        using Transaction = typename WorldData::Scene::Transaction;

        [[nodiscard]]
        explicit SyncWorld(Vector3<u.length> const& blockSize, Solver solver)
            : data_{ blockSize, std::move(solver) }
        {}

        [[nodiscard]]
        Blocks blocks() const {
            return Blocks{ data_ };
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ data_ };
        }

        [[nodiscard]]
        Links links() const {
            return Links{ data_ };
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
