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

#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/Scene.hpp>
#include <gustave/scenes/cuboidGrid/SceneStructure.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/testing/Matchers.hpp>

#include <TestConfig.hpp>

using BlockPosition = Gustave::Scenes::CuboidGrid::BlockPosition;
using Direction = Gustave::Math3d::BasicDirection;
using Scene = Gustave::Scenes::CuboidGrid::Scene<G::libConfig>;
using SceneStructure = Gustave::Scenes::CuboidGrid::SceneStructure<G::libConfig>;
using Transaction = Gustave::Scenes::CuboidGrid::Transaction<G::libConfig>;

static constexpr G::Real<u.density> concreteDensity = 2'400.f * u.density;

static constexpr Gustave::Utils::PointerHash::Equals ptrEquals;

TEST_CASE("Scene::CubeGrid::Scene") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    G::Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;

    Scene scene{blockSize};

    SECTION("::blockSize() const") {
        CHECK(scene.blockSize() == blockSize);
    }

    SECTION("::contactAreaAlong(BasicDirection)") {
        CHECK(scene.contactAreaAlong(Direction::minusX) == 6.f * u.area);
        CHECK(scene.contactAreaAlong(Direction::plusX) == 6.f * u.area);
        CHECK(scene.contactAreaAlong(Direction::minusY) == 3.f * u.area);
        CHECK(scene.contactAreaAlong(Direction::plusY) == 3.f * u.area);
        CHECK(scene.contactAreaAlong(Direction::minusZ) == 2.f * u.area);
        CHECK(scene.contactAreaAlong(Direction::plusZ) == 2.f * u.area);
    }

    SECTION("::thicknessAlong(BasicDirection)") {
        CHECK(scene.thicknessAlong(Direction::minusX) == 1.f * u.length);
        CHECK(scene.thicknessAlong(Direction::plusX) == 1.f * u.length);
        CHECK(scene.thicknessAlong(Direction::minusY) == 2.f * u.length);
        CHECK(scene.thicknessAlong(Direction::plusY) == 2.f * u.length);
        CHECK(scene.thicknessAlong(Direction::minusZ) == 3.f * u.length);
        CHECK(scene.thicknessAlong(Direction::plusZ) == 3.f * u.length);
    }

    SECTION("::modify(Transaction const&)") {
        auto structureOf = [&scene](BlockPosition const& position) -> SceneStructure const& {
            SceneStructure const* result = scene.anyStructureContaining(position);
            REQUIRE(result != nullptr);
            return *result;
        };

        auto getBlockIndex = [](SceneStructure const& structure, BlockPosition const& position) -> G::NodeIndex {
            auto const optBlock = structure.solverIndexOf(position);
            REQUIRE(optBlock);
            return *optBlock;
        };

        auto checkContact = [&scene](SceneStructure const& structure, G::NodeIndex source, G::NodeIndex dest, Direction sourceNormal, G::MaxStress const& maxStress) {
            G::NormalizedVector3 const normal = G::NormalizedVector3::basisVector(sourceNormal);
            G::SolverContact const* selectedLink = nullptr;
            for (G::SolverContact const& link : structure.solverStructure().links()) {
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
            G::Real<u.length> const conductivityFactor = scene.contactAreaAlong(sourceNormal) / scene.thicknessAlong(sourceNormal);
            CHECK_THAT(selectedLink->compressionConductivity(), M::WithinRel(conductivityFactor * maxStress.maxCompressionStress(), epsilon));
            CHECK_THAT(selectedLink->shearConductivity(), M::WithinRel(conductivityFactor * maxStress.maxShearStress(), epsilon));
            CHECK_THAT(selectedLink->tensileConductivity(), M::WithinRel(conductivityFactor * maxStress.maxTensileStress(), epsilon));
        };

        SECTION(" // Transaction{1+}: single foundation") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            auto const result = scene.modify(t);
            CHECK(result.newStructures().size() == 0);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(scene.structures().size() == 0);
            CHECK(scene.anyStructureContaining({ 1,0,0 }) == nullptr);
        }

        SECTION("// Transaction{1+}: single non-foundation") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            auto const result = scene.modify(t);
            CHECK(result.newStructures().size() == 1);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(scene.structures().size() == 1);
            CHECK(scene.blocks().size() == 1);
            SceneStructure const& structure = structureOf({ 1,0,0 });
            CHECK_THAT(result.newStructures(), M::C2::Contains(&structure, ptrEquals));
            G::NodeIndex blockIndex = getBlockIndex(structure, { 1,0,0 });
            G::SolverNode const& solverNode = structure.solverStructure().nodes()[blockIndex];
            CHECK_FALSE(solverNode.isFoundation);
            CHECK(solverNode.mass() == blockMass);
        }

        SECTION("// Transaction{1+} -> Transaction{1-}") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            auto const r1 = scene.modify(t);
            CHECK(r1.newStructures().size() == 0);
            CHECK(r1.deletedStructures().size() == 0);

            t.clear();
            t.removeBlock({ 1,0,0 });
            auto const r2 = scene.modify(t);
            CHECK(r2.newStructures().size() == 0);
            CHECK(r2.deletedStructures().size() == 0);

            CHECK(scene.structures().size() == 0);
            CHECK(scene.blocks().size() == 0);
        }

        SECTION("// Transaction{3+}: 2 structures sharing a foundation.") {
            Transaction t;
            t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
            auto const result = scene.modify(t);
            CHECK(result.newStructures().size() == 2);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(scene.structures().size() == 2);

            {
                SceneStructure const& structureX = structureOf({ 1,0,0 });
                CHECK_THAT(result.newStructures(), M::C2::Contains(&structureX, ptrEquals));
                G::NodeIndex const x1 = getBlockIndex(structureX, { 1,0,0 });
                G::NodeIndex const origin = getBlockIndex(structureX, { 0,0,0 });
                CHECK_FALSE(structureX.contains({ 0,1,0 }));
                checkContact(structureX, origin, x1, Direction::plusX, concrete_20m);
            }

            {
                SceneStructure const& structureY = structureOf({ 0,1,0 });
                CHECK_THAT(result.newStructures(), M::C2::Contains(&structureY, ptrEquals));
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
            auto const result = scene.modify(t);
            CHECK(result.newStructures().size() == 1);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(scene.structures().size() == 1);

            {
                SceneStructure const* structure = scene.anyStructureContaining({ 0,0,0 });
                REQUIRE(structure == nullptr);
            }

            {
                SceneStructure const& structure = structureOf({ 2,0,0 });
                CHECK_THAT(result.newStructures(), M::C2::Contains(&structure, ptrEquals));
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
            auto const result = scene.modify(t);
            CHECK(result.newStructures().size() == 1);
            CHECK(result.deletedStructures().size() == 0);

            CHECK(scene.structures().size() == 1);

            SceneStructure const& structure = structureOf({ 0,0,0 });
            CHECK_THAT(result.newStructures(), M::C2::Contains(&structure, ptrEquals));
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
            auto const r1 = scene.modify(t);
            CHECK(r1.newStructures().size() == 1);
            CHECK(r1.deletedStructures().size() == 0);

            t.clear();
            t.removeBlock({ 0,2,0 });
            auto const r2 = scene.modify(t);
            CHECK(r2.newStructures().size() == 2);
            CHECK_THAT(r2.deletedStructures(), M::C2::UnorderedRangeEquals(r1.newStructures(), ptrEquals));

            CHECK(scene.structures().size() == 2);

            {
                SceneStructure const& structure = structureOf({ 0,0,0 });
                CHECK_THAT(r2.newStructures(), M::C2::Contains(&structure, ptrEquals));
                G::NodeIndex const y0 = getBlockIndex(structure, { 0,0,0 });
                G::NodeIndex const y1 = getBlockIndex(structure, { 0,1,0 });
                CHECK_FALSE(structure.contains({ 0,3,0 }));
                CHECK_FALSE(structure.contains({ 0,4,0 }));
                checkContact(structure, y0, y1, Direction::plusY, concrete_20m);
            }

            {
                SceneStructure const& structure = structureOf({ 0,3,0 });
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
            auto const r1 = scene.modify(t);
            CHECK(r1.newStructures().size() == 2);
            CHECK(r1.deletedStructures().size() == 0);

            t.clear();
            t.addBlock({ { 0,0,2 }, concrete_20m, blockMass, false });
            auto const r2 = scene.modify(t);
            CHECK(r2.newStructures().size() == 1);
            CHECK_THAT(r2.deletedStructures(), M::C2::UnorderedRangeEquals(r1.newStructures(), ptrEquals));

            CHECK(scene.structures().size() == 1);
            SceneStructure const& structure = structureOf({ 0,0,0 });
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
            auto const r1 = scene.modify(t);
            CHECK(r1.newStructures().size() == 2);
            CHECK(r1.deletedStructures().size() == 0);

            SceneStructure const& structureOfX1 = structureOf({ 1,0,0 });
            SceneStructure const& structureOfY1 = structureOf({ 0,1,0 });

            t.clear();
            t.removeBlock({ 0,1,0 });
            auto const r2 = scene.modify(t);
            CHECK(r2.newStructures().size() == 0);
            CHECK(r2.deletedStructures().size() == 1);
            CHECK_THAT(r2.deletedStructures(), M::C2::Contains(&structureOfY1, ptrEquals));

            CHECK(scene.structures().size() == 1);
            CHECK(&structureOfX1 == &structureOf({ 1,0,0 }));
        }

        SECTION("// Transaction{1+}: invalid addition") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            scene.modify(t);

            CHECK_THROWS_AS(scene.modify(t), std::invalid_argument);
        }

        SECTION("// Transaction{1-}: invalid deletion") {
            Transaction t;
            t.removeBlock({ 5,0,0 });
            CHECK_THROWS_AS(scene.modify(t), std::invalid_argument);
        }
    }
}
