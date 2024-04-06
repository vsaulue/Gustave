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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>

namespace gustave::worlds::sync::detail {
    template<cfg::cLibConfig auto libCfg>
    class WorldUpdater {
    public:
        using WorldData = detail::WorldData<libCfg>;

        using SceneStructure = typename WorldData::Scene::StructureReference;
        using SceneTransactionResult = typename WorldData::Scene::TransactionResult;
        using StructureData = typename WorldData::StructureData;
        using Transaction = typename WorldData::Scene::Transaction;

        [[nodiscard]]
        explicit WorldUpdater(WorldData& data)
            : data_{ data }
        {}

        void runTransaction(Transaction const& transaction) {
            SceneTransactionResult const trResult = data_.scene.modify(transaction);
            for (SceneStructure const& sceneStructure : trResult.deletedStructures()) {
                auto node = data_.structures.extract(sceneStructure);
                assert(!node.empty());
                node.mapped()->invalidate();
            }
            for (SceneStructure const& sceneStructure : trResult.newStructures()) {
                std::shared_ptr<StructureData> worldStructure = std::make_shared<StructureData>(data_, sceneStructure);
                auto const result = data_.solver.run(worldStructure->sceneStructure().solverStructurePtr());
                worldStructure->solve(result.solutionPtr());
                auto const insertResult = data_.structures.insert({ worldStructure->sceneStructure(), std::move(worldStructure) });
                assert(insertResult.second);
            }
        }
    private:
        WorldData& data_;
    };
}
