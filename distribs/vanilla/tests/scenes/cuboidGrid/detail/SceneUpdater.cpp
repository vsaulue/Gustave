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

#include <algorithm>
#include <stack>

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneUpdater.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>

#include <TestHelpers.hpp>

namespace CuboidGrid = Gustave::Scenes::CuboidGrid;

using BlockIndex = CuboidGrid::BlockIndex;
using ConstBlockDataReference = CuboidGrid::detail::BlockDataReference<cfg, false>;
using ConstDataNeighbours = CuboidGrid::detail::DataNeighbours<cfg, false>;
using Direction = Gustave::Math3d::BasicDirection;
using SceneData = CuboidGrid::detail::SceneData<cfg>;
using SceneUpdater = CuboidGrid::detail::SceneUpdater<cfg>;
using StructureData = CuboidGrid::detail::StructureData<cfg>;
using Transaction = CuboidGrid::Transaction<cfg>;

using SolverStructure = Gustave::Solvers::Structure<cfg>;
using SolverLink = SolverStructure::Link;
using SolverNode = SolverStructure::Node;

static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Gustave::Utils::PointerHash::Equals ptrEquals;

TEST_CASE("Scenes::CuboidGrid::detail::SceneUpdater") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
    SceneData data{ blockSize };

    auto runTransaction = [&data](Transaction const& transaction) {
        auto oldStructures = data.structures;
        auto result = SceneUpdater{ data }.runTransaction(transaction);
        // Check structure diff of the result.
        for (auto const& deletedStructure : result.removedStructures) {
            auto it = oldStructures.find(deletedStructure);
            REQUIRE(it != oldStructures.end());
            oldStructures.erase(it);
        }
        for (auto const& newStructure : result.newStructures) {
            auto insertResult = oldStructures.insert(newStructure);
            REQUIRE(insertResult.second);
        }
        REQUIRE(data.structures == oldStructures);
        // check structures.
        REQUIRE_FALSE(data.structures.contains(nullptr));
        for (auto const& structure : data.structures) {
            bool hasNonFoundation = false;
            for (auto const& [index,solverIndex] : structure->solverIndices()) {
                ConstBlockDataReference blockRef = data.blocks.find(index);
                REQUIRE(blockRef);
                if (!blockRef.isFoundation()) {
                    hasNonFoundation = true;
                    REQUIRE(blockRef.structure() == structure.get());
                }
            }
            REQUIRE(hasNonFoundation);
        }
        // Check blocks.
        for (auto const& blockData : data.blocks) {
            ConstBlockDataReference blockRef{ &blockData };
            if (blockRef.isFoundation()) {
                REQUIRE(blockRef.structure() == nullptr);
            } else {
                REQUIRE(data.structures.contains(blockRef.structure()));
                for (auto const& neighbour : ConstDataNeighbours{ data.blocks, blockRef.index() }) {
                    auto const& structure = blockRef.structure();
                    REQUIRE(data.structures.contains(structure));
                    REQUIRE(structure->solverIndices().contains(neighbour.block.index()));
                }
            }
        }
        return result;
    };

    SECTION("::runTransaction(Transaction const&)") {
        auto structureOf = [&data](BlockIndex const& index) -> StructureData const& {
            ConstBlockDataReference ref = data.blocks.find(index);
            REQUIRE(ref);
            REQUIRE(ref.structure() != nullptr);
            return *ref.structure();
        };

        auto getSolverIndex = [](StructureData const& structure, BlockIndex const& index) -> NodeIndex {
            auto const optBlock = structure.solverIndexOf(index);
            REQUIRE(optBlock);
            return *optBlock;
        };

        auto checkLink = [&data](StructureData const& structure, NodeIndex source, NodeIndex dest, Direction sourceNormal, MaxStress const& maxStress) {
            NormalizedVector3 const normal = NormalizedVector3::basisVector(sourceNormal);
            SolverLink const* selectedLink = nullptr;
            for (SolverLink const& link : structure.solverStructure().links()) {
                if (link.localNodeId() == source && link.otherNodeId() == dest) {
                    CHECK(link.normal() == normal);
                    selectedLink = &link;
                    break;
                }
                else if (link.localNodeId() == dest && link.otherNodeId() == source) {
                    CHECK(link.normal() == -normal);
                    selectedLink = &link;
                    break;
                }
            }
            REQUIRE(selectedLink != nullptr);
            Real<u.length> const conductivityFactor = data.blocks.contactAreaAlong(sourceNormal) / data.blocks.thicknessAlong(sourceNormal);
            CHECK_THAT(selectedLink->compressionConductivity(), M::WithinRel(conductivityFactor * maxStress.maxCompressionStress(), epsilon));
            CHECK_THAT(selectedLink->shearConductivity(), M::WithinRel(conductivityFactor * maxStress.maxShearStress(), epsilon));
            CHECK_THAT(selectedLink->tensileConductivity(), M::WithinRel(conductivityFactor * maxStress.maxTensileStress(), epsilon));
        };

        SECTION(" // Transaction{1+}: single foundation") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            auto const result = runTransaction(t);
            CHECK(result.newStructures.size() == 0);
            CHECK(result.removedStructures.size() == 0);

            CHECK(data.structures.size() == 0);
            auto const blockRef = data.blocks.find({ 1,0,0 });
            REQUIRE(blockRef);
            CHECK(blockRef.structure() == nullptr);
        }

        SECTION("// Transaction{1+}: single non-foundation") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            auto const result = runTransaction(t);
            CHECK(result.newStructures.size() == 1);
            CHECK(result.removedStructures.size() == 0);

            CHECK(data.structures.size() == 1);
            CHECK(data.blocks.size() == 1);
            StructureData const& structure = structureOf({ 1,0,0 });
            NodeIndex blockIndex = getSolverIndex(structure, { 1,0,0 });
            SolverNode const& solverNode = structure.solverStructure().nodes()[blockIndex];
            CHECK_FALSE(solverNode.isFoundation);
            CHECK(solverNode.mass() == blockMass);
        }

        SECTION("// Transaction{1+} -> Transaction{1-}") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures.size() == 0);
            CHECK(r1.removedStructures.size() == 0);

            t.clear();
            t.removeBlock({ 1,0,0 });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures.size() == 0);
            CHECK(r2.removedStructures.size() == 0);

            CHECK(data.structures.size() == 0);
            CHECK(data.blocks.size() == 0);
        }

        SECTION("// Transaction{3+}: 2 structures sharing a foundation.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            auto const result = runTransaction(t);
            CHECK(result.newStructures.size() == 2);
            CHECK(result.removedStructures.size() == 0);

            CHECK(data.structures.size() == 2);

            {
                StructureData const& structureX = structureOf({ 1,0,0 });
                NodeIndex const x1 = getSolverIndex(structureX, { 1,0,0 });
                NodeIndex const origin = getSolverIndex(structureX, { 0,0,0 });
                CHECK_FALSE(structureX.contains({ 0,1,0 }));
                checkLink(structureX, origin, x1, Direction::plusX, concrete_20m);
            }

            {
                StructureData const& structureY = structureOf({ 0,1,0 });
                NodeIndex y1 = getSolverIndex(structureY, { 0,1,0 });
                NodeIndex origin = getSolverIndex(structureY, { 0,0,0 });
                CHECK_FALSE(structureY.contains({ 1,0,0 }));
                checkLink(structureY, origin, y1, Direction::plusY, concrete_20m);
            }
        }

        SECTION("// Transaction{3+}: 2 adjacent foundations, 1 non-fondation.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {2,0,0}, concrete_20m, blockMass, false });
            auto const result = runTransaction(t);
            CHECK(result.newStructures.size() == 1);
            CHECK(result.removedStructures.size() == 0);

            CHECK(data.structures.size() == 1);

            {
                auto const blockRef = data.blocks.find({ 0,0,0 });
                REQUIRE(blockRef);
                REQUIRE(blockRef.structure() == nullptr);
            }

            {
                StructureData const& structure = structureOf({ 2,0,0 });
                NodeIndex const x1 = getSolverIndex(structure, { 1,0,0 });
                NodeIndex const x2 = getSolverIndex(structure, { 2,0,0 });
                CHECK_FALSE(structure.contains({ 0,0,0 }));
                checkLink(structure, x1, x2, Direction::plusX, concrete_20m);
            }
        }

        SECTION("// Transaction{5+}: pillar") {
            Transaction t;
            for (int i = 0; i < 5; ++i) {
                t.addBlock({ {0,i,0}, concrete_20m, blockMass, i == 0 });
            }
            auto const result = runTransaction(t);
            CHECK(result.newStructures.size() == 1);
            CHECK(result.removedStructures.size() == 0);

            CHECK(data.structures.size() == 1);

            StructureData const& structure = structureOf({ 0,1,0 });
            for (int i = 0; i < 4; ++i) {
                NodeIndex const bottom = getSolverIndex(structure, { 0,i,0 });
                NodeIndex const top = getSolverIndex(structure, { 0,i + 1,0 });
                checkLink(structure, bottom, top, Direction::plusY, concrete_20m);
            }
        }

        SECTION(" Transaction{5+} -> Transaction{1-}: split structure") {
            Transaction t;
            for (int i = 0; i < 5; ++i) {
                t.addBlock({ {0,i,0}, concrete_20m, blockMass, false });
            }
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures.size() == 1);
            CHECK(r1.removedStructures.size() == 0);

            t.clear();
            t.removeBlock({ 0,2,0 });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures.size() == 2);
            CHECK_THAT(r2.removedStructures, M::C2::UnorderedRangeEquals(r1.newStructures, ptrEquals));

            CHECK(data.structures.size() == 2);

            {
                StructureData const& structure = structureOf({ 0,0,0 });
                CHECK_THAT(r2.newStructures, M::C2::Contains(&structure, ptrEquals));
                NodeIndex const y0 = getSolverIndex(structure, { 0,0,0 });
                NodeIndex const y1 = getSolverIndex(structure, { 0,1,0 });
                CHECK_FALSE(structure.contains({ 0,3,0 }));
                CHECK_FALSE(structure.contains({ 0,4,0 }));
                checkLink(structure, y0, y1, Direction::plusY, concrete_20m);
            }

            {
                StructureData const& structure = structureOf({ 0,3,0 });
                CHECK_THAT(r2.newStructures, M::C2::Contains(&structure, ptrEquals));
                NodeIndex const y3 = getSolverIndex(structure, { 0,3,0 });
                NodeIndex const y4 = getSolverIndex(structure, { 0,4,0 });
                CHECK_FALSE(structure.contains({ 0,0,0 }));
                CHECK_FALSE(structure.contains({ 0,1,0 }));
                checkLink(structure, y3, y4, Direction::plusY, concrete_20m);
            }
        }

        SECTION("// Transaction{4+} -> Transaction{1+}: new block causes 2 structures to merge.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {0,0,1}, concrete_20m, blockMass, false });
            t.addBlock({ {0,0,3}, concrete_20m, blockMass, false });
            t.addBlock({ {0,0,4}, concrete_20m, blockMass, true });
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures.size() == 2);
            CHECK(r1.removedStructures.size() == 0);

            t.clear();
            t.addBlock({ { 0,0,2 }, concrete_20m, blockMass, false });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures.size() == 1);
            CHECK_THAT(r2.removedStructures, M::C2::UnorderedRangeEquals(r1.newStructures, ptrEquals));

            CHECK(data.structures.size() == 1);
            StructureData const& structure = structureOf({ 0,0,1 });
            CHECK_THAT(r2.newStructures, M::C2::Contains(&structure, ptrEquals));
            NodeIndex const z0 = getSolverIndex(structure, { 0,0,0 });
            NodeIndex const z1 = getSolverIndex(structure, { 0,0,1 });
            NodeIndex const z2 = getSolverIndex(structure, { 0,0,2 });
            NodeIndex const z3 = getSolverIndex(structure, { 0,0,3 });
            NodeIndex const z4 = getSolverIndex(structure, { 0,0,4 });
            checkLink(structure, z0, z1, Direction::plusZ, concrete_20m);
            checkLink(structure, z1, z2, Direction::plusZ, concrete_20m);
            checkLink(structure, z2, z3, Direction::plusZ, concrete_20m);
            checkLink(structure, z3, z4, Direction::plusZ, concrete_20m);
        }

        SECTION("// Transaction{3+} -> Transaction{1-}: unmodified structure is not invalidated or rebuilt.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures.size() == 2);
            CHECK(r1.removedStructures.size() == 0);

            StructureData const& structureOfX1 = structureOf({ 1,0,0 });
            StructureData const& structureOfY1 = structureOf({ 0,1,0 });

            t.clear();
            t.removeBlock({ 0,1,0 });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures.size() == 0);
            CHECK(r2.removedStructures.size() == 1);
            CHECK_THAT(r2.removedStructures, M::C2::Contains(&structureOfY1, ptrEquals));

            CHECK(data.structures.size() == 1);
            CHECK(&structureOfX1 == &structureOf({ 1,0,0 }));
        }

        SECTION("// Transaction{1+}: invalid addition") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            runTransaction(t);

            CHECK_THROWS_AS(SceneUpdater{ data }.runTransaction(t), std::invalid_argument);
        }

        SECTION("// Transaction{1-}: invalid deletion") {
            Transaction t;
            t.removeBlock({ 5,0,0 });
            CHECK_THROWS_AS(SceneUpdater{ data }.runTransaction(t), std::invalid_argument);
        }
    }
}
