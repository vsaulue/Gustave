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

#include <algorithm>
#include <stack>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockDataReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/Transaction.hpp>

#include <TestHelpers.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using BlockIndex = cuboid::BlockIndex;
using ConstBlockDataReference = cuboid::detail::BlockDataReference<libCfg, false>;
using ConstDataNeighbours = cuboid::detail::DataNeighbours<libCfg, false>;
using Direction = gustave::math3d::BasicDirection;
using LinkIndices = ConstBlockDataReference::LinkIndices;
using SceneData = cuboid::detail::SceneData<libCfg>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg>;
using StructureData = cuboid::detail::StructureData<libCfg>;
using Transaction = cuboid::Transaction<libCfg>;

using SolverStructure = gustave::core::solvers::Structure<libCfg>;
using SolverLink = SolverStructure::Link;
using SolverNode = SolverStructure::Node;

using StructureIndex = StructureData::StructureIndex;

template<typename Ptr>
using PtrSet = gustave::utils::PointerHash::Set<Ptr>;

static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr gustave::utils::PointerHash::Equals ptrEquals;

TEST_CASE("core::scenes::cuboidGridScene::detail::SceneUpdater") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
    SceneData data{ blockSize };

    auto getStructureIds = [&]() {
        std::unordered_set<StructureIndex> result;
        for (auto const& structure : data.structures) {
            auto const insertRes = result.insert(structure->index());
            REQUIRE(insertRes.second);
        }
        return result;
    };

    auto runTransaction = [&](Transaction const& transaction) {
        PtrSet<std::shared_ptr<StructureData const>> oldStructures = { data.structures.begin(), data.structures.end() };
        auto result = SceneUpdater{ data }.runTransaction(transaction);
        // Check structure diff of the result.
        auto const newStructureIds = getStructureIds();
        for (auto const& deletedStructure : result.removedStructures) {
            auto it = oldStructures.find(deletedStructure);
            REQUIRE(it != oldStructures.end());
            REQUIRE_FALSE(newStructureIds.contains(deletedStructure->index()));
            oldStructures.erase(it);
        }
        for (auto const& newStructure : result.newStructures) {
            auto insertResult = oldStructures.insert(newStructure);
            REQUIRE(insertResult.second);
        }
        REQUIRE_THAT(data.structures, matchers::c2::UnorderedRangeEquals(oldStructures));
        // check structures.
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
            StructureData const* blockStruct = blockRef.structure();
            // structure
            if (blockRef.isFoundation()) {
                REQUIRE(blockStruct == nullptr);
            } else {
                REQUIRE(data.structures.contains(blockStruct));
                for (auto const& neighbour : ConstDataNeighbours{ data.blocks, blockRef.index() }) {
                    REQUIRE(data.structures.contains(blockStruct));
                    REQUIRE(blockStruct->solverIndices().contains(neighbour.block.index()));
                }
            }
            // linkIndices
            auto checkNeighbour = [&](Direction direction, LinkIndex (LinkIndices::*indexField)) {
                auto const neighbourIndex = blockRef.index().neighbourAlong(direction);
                if (neighbourIndex) {
                    auto const neighbourRef = data.blocks.find(*neighbourIndex);
                    if (neighbourRef) {
                        if (!blockRef.isFoundation() || !neighbourRef.isFoundation()) {
                            StructureData const* structure = (blockStruct != nullptr) ? blockStruct : neighbourRef.structure();
                            REQUIRE(structure != nullptr);
                            LinkIndex const& linkId = blockRef.linkIndices().*indexField;
                            SolverLink const& link = structure->solverStructure().links().at(linkId);
                            CHECK(link.localNodeId() == *structure->solverIndexOf(blockRef.index()));
                            CHECK(link.otherNodeId() == *structure->solverIndexOf(neighbourRef.index()));
                        }
                    }
                }
            };
            checkNeighbour(Direction::plusX(), &LinkIndices::plusX);
            checkNeighbour(Direction::plusY(), &LinkIndices::plusY);
            checkNeighbour(Direction::plusZ(), &LinkIndices::plusZ);
        }
        return result;
    };

    SECTION(".runTransaction(Transaction const&)") {
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

        auto checkLink = [&data](StructureData const& structure, NodeIndex source, NodeIndex dest, Direction sourceNormal, PressureStress const& maxStress) {
            NormalizedVector3 const normal = NormalizedVector3::basisVector(sourceNormal);
            SolverLink const* selectedLink = nullptr;
            for (SolverLink const& link : structure.solverStructure().links()) {
                if (link.localNodeId() == source && link.otherNodeId() == dest) {
                    CHECK(link.normal() == normal);
                    selectedLink = &link;
                    break;
                } else if (link.localNodeId() == dest && link.otherNodeId() == source) {
                    CHECK(link.normal() == -normal);
                    selectedLink = &link;
                    break;
                }
            }
            REQUIRE(selectedLink != nullptr);
            Real<u.length> const conductivityFactor = data.blocks.contactAreaAlong(sourceNormal) / data.blocks.thicknessAlong(sourceNormal);
            CHECK_THAT(selectedLink->conductivity().compression(), matchers::WithinRel(conductivityFactor * maxStress.compression(), epsilon));
            CHECK_THAT(selectedLink->conductivity().shear(), matchers::WithinRel(conductivityFactor * maxStress.shear(), epsilon));
            CHECK_THAT(selectedLink->conductivity().tensile(), matchers::WithinRel(conductivityFactor * maxStress.tensile(), epsilon));
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
            CHECK(structure.index() == 0);
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

            StructureData const& structureX = structureOf({ 1,0,0 });
            {
                NodeIndex const x1 = getSolverIndex(structureX, { 1,0,0 });
                NodeIndex const origin = getSolverIndex(structureX, { 0,0,0 });
                CHECK_FALSE(structureX.contains({ 0,1,0 }));
                checkLink(structureX, origin, x1, Direction::plusX(), concrete_20m);
            }

            StructureData const& structureY = structureOf({ 0,1,0 });
            {
                NodeIndex y1 = getSolverIndex(structureY, { 0,1,0 });
                NodeIndex origin = getSolverIndex(structureY, { 0,0,0 });
                CHECK_FALSE(structureY.contains({ 1,0,0 }));
                checkLink(structureY, origin, y1, Direction::plusY(), concrete_20m);
            }

            CHECK(structureX.index() != structureY.index());
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
                CHECK(structure.index() == 0);
                NodeIndex const x1 = getSolverIndex(structure, { 1,0,0 });
                NodeIndex const x2 = getSolverIndex(structure, { 2,0,0 });
                CHECK_FALSE(structure.contains({ 0,0,0 }));
                checkLink(structure, x1, x2, Direction::plusX(), concrete_20m);
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
            CHECK(structure.index() == 0);
            for (int i = 0; i < 4; ++i) {
                NodeIndex const bottom = getSolverIndex(structure, { 0,i,0 });
                NodeIndex const top = getSolverIndex(structure, { 0,i + 1,0 });
                checkLink(structure, bottom, top, Direction::plusY(), concrete_20m);
            }
        }

        SECTION("// Transaction{5+} -> Transaction{1-}: split structure") {
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
            CHECK_THAT(r2.removedStructures, matchers::c2::UnorderedRangeEquals(r1.newStructures, ptrEquals));

            CHECK(data.structures.size() == 2);

            StructureData const& s0 = structureOf({ 0,0,0 });
            CHECK(s0.index() != 0);
            {
                CHECK_THAT(r2.newStructures, matchers::c2::Contains(&s0, ptrEquals));
                NodeIndex const y0 = getSolverIndex(s0, { 0,0,0 });
                NodeIndex const y1 = getSolverIndex(s0, { 0,1,0 });
                CHECK_FALSE(s0.contains({ 0,3,0 }));
                CHECK_FALSE(s0.contains({ 0,4,0 }));
                checkLink(s0, y0, y1, Direction::plusY(), concrete_20m);
            }

            StructureData const& s3 = structureOf({ 0,3,0 });
            CHECK(s3.index() != 0);
            {
                CHECK_THAT(r2.newStructures, matchers::c2::Contains(&s3, ptrEquals));
                NodeIndex const y3 = getSolverIndex(s3, { 0,3,0 });
                NodeIndex const y4 = getSolverIndex(s3, { 0,4,0 });
                CHECK_FALSE(s3.contains({ 0,0,0 }));
                CHECK_FALSE(s3.contains({ 0,1,0 }));
                checkLink(s3, y3, y4, Direction::plusY(), concrete_20m);
            }

            CHECK(s0.index() != s3.index());
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
            CHECK_THAT(r2.removedStructures, matchers::c2::UnorderedRangeEquals(r1.newStructures, ptrEquals));

            CHECK(data.structures.size() == 1);
            StructureData const& structure = structureOf({ 0,0,1 });
            CHECK(structure.index() == 2);
            CHECK_THAT(r2.newStructures, matchers::c2::Contains(&structure, ptrEquals));
            NodeIndex const z0 = getSolverIndex(structure, { 0,0,0 });
            NodeIndex const z1 = getSolverIndex(structure, { 0,0,1 });
            NodeIndex const z2 = getSolverIndex(structure, { 0,0,2 });
            NodeIndex const z3 = getSolverIndex(structure, { 0,0,3 });
            NodeIndex const z4 = getSolverIndex(structure, { 0,0,4 });
            checkLink(structure, z0, z1, Direction::plusZ(), concrete_20m);
            checkLink(structure, z1, z2, Direction::plusZ(), concrete_20m);
            checkLink(structure, z2, z3, Direction::plusZ(), concrete_20m);
            checkLink(structure, z3, z4, Direction::plusZ(), concrete_20m);
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
            CHECK_THAT(r2.removedStructures, matchers::c2::Contains(&structureOfY1, ptrEquals));

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
