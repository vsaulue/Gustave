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

#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/DataNeighbours.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneUpdater.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/scenes/cuboidGrid/TransactionResult.hpp>

#include <TestConfig.hpp>

namespace CuboidGrid = Gustave::Scenes::CuboidGrid;

using BlockPosition = CuboidGrid::BlockPosition;
using ConstBlockDataReference = CuboidGrid::detail::BlockDataReference<G::libConfig, false>;
using ConstDataNeighbours = CuboidGrid::detail::DataNeighbours<G::libConfig, false>;
using Direction = Gustave::Math3d::BasicDirection;
using SceneData = CuboidGrid::detail::SceneData<G::libConfig>;
using SceneUpdater = CuboidGrid::detail::SceneUpdater<G::libConfig>;
using StructureData = CuboidGrid::detail::StructureData<G::libConfig>;
using Transaction = CuboidGrid::Transaction<G::libConfig>;
using TransactionResult = CuboidGrid::TransactionResult<G::libConfig>;

static constexpr G::Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Gustave::Utils::PointerHash::Equals ptrEquals;

TEST_CASE("Scenes::CuboidGrid::detail::SceneUpdater") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    G::Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
    SceneData data{ blockSize };

    auto runTransaction = [&data](Transaction const& transaction) -> TransactionResult {
        auto oldStructures = data.structures;
        TransactionResult result = SceneUpdater{ data }.runTransaction(transaction);
        // Check structure diff of the TransactionResult.
        for (auto const& deletedStructure : result.deletedStructures()) {
            auto it = oldStructures.find(deletedStructure);
            REQUIRE(it != oldStructures.end());
            oldStructures.erase(it);
        }
        for (auto const& newStructure : result.newStructures()) {
            auto insertResult = oldStructures.insert(newStructure);
            REQUIRE(insertResult.second);
        }
        REQUIRE(data.structures == oldStructures);
        // check structures.
        REQUIRE_FALSE(data.structures.contains(nullptr));
        for (auto const& structure : data.structures) {
            bool hasNonFoundation = false;
            for (auto const& [position,solverIndex] : structure->solverIndices()) {
                ConstBlockDataReference blockRef = data.blocks.find(position);
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
                for (auto const& neighbour : ConstDataNeighbours{ data.blocks, blockRef.position() }) {
                    auto const& structure = blockRef.structure();
                    REQUIRE(data.structures.contains(structure));
                    REQUIRE(structure->solverIndices().contains(neighbour.block.position()));
                }
            }
        }
        return result;
    };

    SECTION("::runTransaction(Transaction const&)") {
        auto structureOf = [&data](BlockPosition const& position) -> StructureData const& {
            ConstBlockDataReference ref = data.blocks.find(position);
            REQUIRE(ref);
            REQUIRE(ref.structure() != nullptr);
            return *ref.structure();
        };

        auto getBlockIndex = [](StructureData const& structure, BlockPosition const& position) -> G::NodeIndex {
            auto const optBlock = structure.solverIndexOf(position);
            REQUIRE(optBlock);
            return *optBlock;
        };

        auto checkContact = [&data](StructureData const& structure, G::NodeIndex source, G::NodeIndex dest, Direction sourceNormal, G::MaxStress const& maxStress) {
            G::NormalizedVector3 const normal = G::NormalizedVector3::basisVector(sourceNormal);
            G::SolverContact const* selectedLink = nullptr;
            for (G::SolverContact const& link : structure.solverStructure().links()) {
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
            G::Real<u.length> const conductivityFactor = data.blocks.contactAreaAlong(sourceNormal) / data.blocks.thicknessAlong(sourceNormal);
            CHECK_THAT(selectedLink->compressionConductivity(), M::WithinRel(conductivityFactor * maxStress.maxCompressionStress(), epsilon));
            CHECK_THAT(selectedLink->shearConductivity(), M::WithinRel(conductivityFactor * maxStress.maxShearStress(), epsilon));
            CHECK_THAT(selectedLink->tensileConductivity(), M::WithinRel(conductivityFactor * maxStress.maxTensileStress(), epsilon));
        };

        SECTION(" // Transaction{1+}: single foundation") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            auto const result = runTransaction(t);
            CHECK(result.newStructures().size() == 0);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(data.structures.size() == 0);
            auto const blockRef = data.blocks.find({ 1,0,0 });
            REQUIRE(blockRef);
            CHECK(blockRef.structure() == nullptr);
        }

        SECTION("// Transaction{1+}: single non-foundation") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            auto const result = runTransaction(t);
            CHECK(result.newStructures().size() == 1);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(data.structures.size() == 1);
            CHECK(data.blocks.size() == 1);
            StructureData const& structure = structureOf({ 1,0,0 });
            G::NodeIndex blockIndex = getBlockIndex(structure, { 1,0,0 });
            G::SolverNode const& solverNode = structure.solverStructure().nodes()[blockIndex];
            CHECK_FALSE(solverNode.isFoundation);
            CHECK(solverNode.mass() == blockMass);
        }

        SECTION("// Transaction{1+} -> Transaction{1-}") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures().size() == 0);
            CHECK(r1.deletedStructures().size() == 0);

            t.clear();
            t.removeBlock({ 1,0,0 });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures().size() == 0);
            CHECK(r2.deletedStructures().size() == 0);

            CHECK(data.structures.size() == 0);
            CHECK(data.blocks.size() == 0);
        }

        SECTION("// Transaction{3+}: 2 structures sharing a foundation.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            auto const result = runTransaction(t);
            CHECK(result.newStructures().size() == 2);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(data.structures.size() == 2);

            {
                StructureData const& structureX = structureOf({ 1,0,0 });
                G::NodeIndex const x1 = getBlockIndex(structureX, { 1,0,0 });
                G::NodeIndex const origin = getBlockIndex(structureX, { 0,0,0 });
                CHECK_FALSE(structureX.contains({ 0,1,0 }));
                checkContact(structureX, origin, x1, Direction::plusX, concrete_20m);
            }

            {
                StructureData const& structureY = structureOf({ 0,1,0 });
                G::NodeIndex y1 = getBlockIndex(structureY, { 0,1,0 });
                G::NodeIndex origin = getBlockIndex(structureY, { 0,0,0 });
                CHECK_FALSE(structureY.contains({ 1,0,0 }));
                checkContact(structureY, origin, y1, Direction::plusY, concrete_20m);
            }
        }

        SECTION("// Transaction{3+}: 2 adjacent foundations, 1 non-fondation.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {2,0,0}, concrete_20m, blockMass, false });
            auto const result = runTransaction(t);
            CHECK(result.newStructures().size() == 1);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(data.structures.size() == 1);

            {
                auto const blockRef = data.blocks.find({ 0,0,0 });
                REQUIRE(blockRef);
                REQUIRE(blockRef.structure() == nullptr);
            }

            {
                StructureData const& structure = structureOf({ 2,0,0 });
                G::NodeIndex const x1 = getBlockIndex(structure, { 1,0,0 });
                G::NodeIndex const x2 = getBlockIndex(structure, { 2,0,0 });
                CHECK_FALSE(structure.contains({ 0,0,0 }));
                checkContact(structure, x1, x2, Direction::plusX, concrete_20m);
            }
        }

        SECTION("// Transaction{5+}: pillar") {
            Transaction t;
            for (int i = 0; i < 5; ++i) {
                t.addBlock({ {0,i,0}, concrete_20m, blockMass, i == 0 });
            }
            auto const result = runTransaction(t);
            CHECK(result.newStructures().size() == 1);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(data.structures.size() == 1);

            StructureData const& structure = structureOf({ 0,1,0 });
            for (int i = 0; i < 4; ++i) {
                G::NodeIndex const bottom = getBlockIndex(structure, { 0,i,0 });
                G::NodeIndex const top = getBlockIndex(structure, { 0,i + 1,0 });
                checkContact(structure, bottom, top, Direction::plusY, concrete_20m);
            }
        }

        SECTION(" Transaction{5+} -> Transaction{1-}: split structure") {
            Transaction t;
            for (int i = 0; i < 5; ++i) {
                t.addBlock({ {0,i,0}, concrete_20m, blockMass, false });
            }
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures().size() == 1);
            CHECK(r1.deletedStructures().size() == 0);

            t.clear();
            t.removeBlock({ 0,2,0 });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures().size() == 2);
            CHECK_THAT(r2.deletedStructures(), M::C2::UnorderedRangeEquals(r1.newStructures(), ptrEquals));

            CHECK(data.structures.size() == 2);

            {
                StructureData const& structure = structureOf({ 0,0,0 });
                CHECK_THAT(r2.newStructures(), M::C2::Contains(&structure, ptrEquals));
                G::NodeIndex const y0 = getBlockIndex(structure, { 0,0,0 });
                G::NodeIndex const y1 = getBlockIndex(structure, { 0,1,0 });
                CHECK_FALSE(structure.contains({ 0,3,0 }));
                CHECK_FALSE(structure.contains({ 0,4,0 }));
                checkContact(structure, y0, y1, Direction::plusY, concrete_20m);
            }

            {
                StructureData const& structure = structureOf({ 0,3,0 });
                CHECK_THAT(r2.newStructures(), M::C2::Contains(&structure, ptrEquals));
                G::NodeIndex const y3 = getBlockIndex(structure, { 0,3,0 });
                G::NodeIndex const y4 = getBlockIndex(structure, { 0,4,0 });
                CHECK_FALSE(structure.contains({ 0,0,0 }));
                CHECK_FALSE(structure.contains({ 0,1,0 }));
                checkContact(structure, y3, y4, Direction::plusY, concrete_20m);
            }
        }

        SECTION("// Transaction{4+} -> Transaction{1+}: new block causes 2 structures to merge.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {0,0,1}, concrete_20m, blockMass, false });
            t.addBlock({ {0,0,3}, concrete_20m, blockMass, false });
            t.addBlock({ {0,0,4}, concrete_20m, blockMass, true });
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures().size() == 2);
            CHECK(r1.deletedStructures().size() == 0);

            t.clear();
            t.addBlock({ { 0,0,2 }, concrete_20m, blockMass, false });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures().size() == 1);
            CHECK_THAT(r2.deletedStructures(), M::C2::UnorderedRangeEquals(r1.newStructures(), ptrEquals));

            CHECK(data.structures.size() == 1);
            StructureData const& structure = structureOf({ 0,0,1 });
            CHECK_THAT(r2.newStructures(), M::C2::Contains(&structure, ptrEquals));
            G::NodeIndex const z0 = getBlockIndex(structure, { 0,0,0 });
            G::NodeIndex const z1 = getBlockIndex(structure, { 0,0,1 });
            G::NodeIndex const z2 = getBlockIndex(structure, { 0,0,2 });
            G::NodeIndex const z3 = getBlockIndex(structure, { 0,0,3 });
            G::NodeIndex const z4 = getBlockIndex(structure, { 0,0,4 });
            checkContact(structure, z0, z1, Direction::plusZ, concrete_20m);
            checkContact(structure, z1, z2, Direction::plusZ, concrete_20m);
            checkContact(structure, z2, z3, Direction::plusZ, concrete_20m);
            checkContact(structure, z3, z4, Direction::plusZ, concrete_20m);
        }

        SECTION("// Transaction{3+} -> Transaction{1-}: unmodified structure is not invalidated or rebuilt.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            auto const r1 = runTransaction(t);
            CHECK(r1.newStructures().size() == 2);
            CHECK(r1.deletedStructures().size() == 0);

            StructureData const& structureOfX1 = structureOf({ 1,0,0 });
            StructureData const& structureOfY1 = structureOf({ 0,1,0 });

            t.clear();
            t.removeBlock({ 0,1,0 });
            auto const r2 = runTransaction(t);
            CHECK(r2.newStructures().size() == 0);
            CHECK(r2.deletedStructures().size() == 1);
            CHECK_THAT(r2.deletedStructures(), M::C2::Contains(&structureOfY1, ptrEquals));

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
