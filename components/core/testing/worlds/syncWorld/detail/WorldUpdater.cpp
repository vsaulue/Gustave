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

#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldUpdater.hpp>

#include <TestHelpers.hpp>

using StructureData = gustave::core::worlds::syncWorld::detail::StructureData<libCfg>;
using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::core::worlds::syncWorld::detail::WorldUpdater<libCfg>;

using Solver = WorldData::Solver;
using Transaction = WorldUpdater::Transaction;
using BlockIndex = WorldData::Scene::BlockIndex;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ Solver::Config{ g, solverPrecision } };
    return WorldData{ blockSize, std::move(solver) };
}

TEST_CASE("core::worlds::syncWorld::detail::WorldUpdater") {
    WorldData world = makeWorld();

    auto runTransaction = [&](Transaction const& transaction) {
        auto oldStructures = world.structures;
        auto const result = WorldUpdater{ world }.runTransaction(transaction);
        // Check new structures are solved.
        for (auto const& sceneStructure : world.scene.structures()) {
            auto const it = world.structures.find(sceneStructure.index());
            REQUIRE(it != world.structures.end());
            auto const& worldStructure = it->second;
            CHECK(worldStructure->state() == StructureData::State::Solved);
        }
        // Check structure removed from scenes are invalidated.
        for (auto const& [structId, worldStruct] : oldStructures) {
            if (worldStruct->sceneStructure().isValid()) {
                CHECK(worldStruct->state() == StructureData::State::Solved);
            } else {
                CHECK(worldStruct->state() == StructureData::State::Invalid);
            }
        }
        return result;
    };

    auto sceneStructureOf = [&](BlockIndex const& blockIndex) {
        auto const sceneStructs = world.scene.blocks().at(blockIndex).structures();
        REQUIRE(sceneStructs.size() == 1);
        return sceneStructs[0].asImmutable();
    };

    auto checkForce = [&](BlockIndex const& to, BlockIndex const& from, Vector3<u.force> const& expectedForce) {
        auto const worldStruct = world.structures.at(sceneStructureOf(from).index());

        auto const idTo = worldStruct->sceneStructure().solverIndexOf(to);
        auto const idFrom = worldStruct->sceneStructure().solverIndexOf(from);
        REQUIRE(idTo);
        REQUIRE(idFrom);
        auto const force = worldStruct->solution().nodes().at(*idTo).forceVectorFrom(*idFrom);
        CHECK_THAT(force, matchers::WithinRel(expectedForce, solverPrecision));
    };

    SECTION(".runTransaction(Transaction const&)") {
        SECTION("// Transaction{3+}: simple tower") {
            WorldUpdater::Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
            auto const trResult = runTransaction(t);

            REQUIRE(trResult.deletedStructures().size() == 0);
            REQUIRE(trResult.newStructures().size() == 1);

            checkForce({ 0,0,0 }, { 0,1,0 }, 2.f * blockMass * g);
            checkForce({ 0,1,0 }, { 0,2,0 }, blockMass * g);
        }

        SECTION("// Transaction{3+} -> Transaction{1-}: edit structure") {
            WorldUpdater::Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
            auto const trRes1 = runTransaction(t);

            CHECK(trRes1.deletedStructures().size() == 0);
            REQUIRE(trRes1.newStructures().size() == 1);
            auto const oldStruct = world.structures.at(trRes1.newStructures().at(0));
            REQUIRE(oldStruct->state() == StructureData::State::Solved);

            t.clear();
            t.removeBlock({ 0,2,0 });
            auto const trRes2 = runTransaction(t);

            REQUIRE(trRes2.deletedStructures().size() == 1);
            CHECK(trRes2.deletedStructures().at(0) == oldStruct->sceneStructure().index());
            CHECK(trRes2.newStructures().size() == 1);
            CHECK(oldStruct->state() == StructureData::State::Invalid);
            checkForce({ 0,0,0 }, { 0,1,0 }, blockMass * g);
        }
    }
}
