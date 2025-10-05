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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>

namespace gustave::core::worlds::syncWorld::detail {
    template<cfg::cLibConfig auto libCfg>
    class WorldUpdater {
    public:
        using WorldData = detail::WorldData<libCfg>;

        using SceneStructure = typename WorldData::Scene::StructureReference;
        using TransactionResult = typename WorldData::Scene::TransactionResult;
        using StructureData = typename WorldData::StructureData;
        using Transaction = typename WorldData::Scene::Transaction;

        [[nodiscard]]
        explicit WorldUpdater(WorldData& data)
            : data_{ data }
        {}

        TransactionResult runTransaction(Transaction const& transaction) {
            TransactionResult const result = data_.scene.modify(transaction);
            for (auto const& structureId : result.deletedStructures()) {
                auto node = data_.structures.extract(structureId);
                assert(!node.empty());
                node.mapped()->invalidate();
            }
            for (auto const& structureId : result.newStructures()) {
                std::shared_ptr<StructureData> worldStructure = std::make_shared<StructureData>(data_, data_.scene.structures().at(structureId));
                auto const result = data_.solver.run(worldStructure->sceneStructure().solverStructurePtr());
                worldStructure->solve(result.solutionPtr());
                [[maybe_unused]] auto const insertResult = data_.structures.insert({ structureId, std::move(worldStructure) });
                assert(insertResult.second);
            }
            return result;
        }
    private:
        WorldData& data_;
    };
}
