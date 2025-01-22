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

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/Transaction.hpp>

#include <TestHelpers.hpp>

using ContactReference = gustave::core::scenes::cuboidGridScene::ContactReference<libCfg>;
using SceneData = gustave::core::scenes::cuboidGridScene::detail::SceneData<libCfg>;
using SceneUpdater = gustave::core::scenes::cuboidGridScene::detail::SceneUpdater<libCfg>;
using StructureReference = gustave::core::scenes::cuboidGridScene::StructureReference<libCfg>;

using BlockIndex = ContactReference::BlockIndex;
using BlockReference = ContactReference::BlockReference;
using ContactIndex = ContactReference::ContactIndex;
using Direction = ContactIndex::Direction;
using Transaction = gustave::core::scenes::cuboidGridScene::Transaction<libCfg>;

TEST_CASE("core::scenes::cuboidGridScene::ContactReference") {
    auto const blockSize = vector3(2.f, 3.f, 1.f, u.length);
    SceneData scene{ blockSize };

    PressureStress highTensile{
        1'000'000.f * u.pressure,
        15'000'000.f * u.pressure,
        20'000'000.f * u.pressure,
    };

    Transaction t;
    auto newBlock = [&](BlockIndex const& index, PressureStress const& material, bool isFoundation) {
        t.addBlock({ index, material, 1000.f * u.mass, isFoundation });
        return BlockReference{ scene, index };
    };
    BlockReference b122 = newBlock({ 1,2,2 }, highTensile, false);
    BlockReference b222 = newBlock({ 2,2,2 }, concrete_20m, true);
    newBlock({ 2,3,2 }, concrete_20m, true);
    newBlock({ 7,7,7 }, concrete_20m, false);
    newBlock({ 6,7,7 }, concrete_20m, true);
    newBlock({ 7,6,7 }, concrete_20m, false);
    newBlock({ 7,7,6 }, concrete_20m, false);
    SceneUpdater{ scene }.runTransaction(t);

    auto contactReference = [&](BlockIndex const& localBlockId, Direction direction) {
        return ContactReference{ scene, ContactIndex{ localBlockId, direction } };
    };

    auto invalidContact = contactReference({ 0,0,0 }, Direction::minusZ());

    auto structureRefAt = [&](BlockIndex const& blockId) -> StructureReference {
        auto blockDataRef = scene.blocks.find(blockId);
        REQUIRE(blockDataRef);
        auto rawStruct = blockDataRef.structure();
        REQUIRE(rawStruct != nullptr);
        auto it = scene.structures.find(rawStruct);
        REQUIRE(it != scene.structures.end());
        return StructureReference{ *it };
        };

    SECTION(".area()") {
        ContactReference contact = contactReference({ 2,2,2 }, Direction::minusX());
        CHECK(contact.area() == 3.f * u.area);
    }

    SECTION(".index()") {
        ContactIndex id{ { 2,2,2 }, Direction::minusX() };
        ContactReference contact{ scene, id };
        CHECK(contact.index() == id);
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            ContactReference contact = contactReference({ 2,2,2 }, Direction::minusX());
            CHECK(contact.isValid());
        }

        SECTION("// false: no local block") {
            ContactReference contact = contactReference({ 2,1,2 }, Direction::plusY());
            CHECK_FALSE(contact.isValid());
        }

        SECTION("// false: no other block") {
            ContactReference contact = contactReference({ 2,2,2 }, Direction::minusY());
            CHECK_FALSE(contact.isValid());
        }

        SECTION("// false: both foundations") {
            ContactReference contact = contactReference({ 2,2,2 }, Direction::plusY());
            CHECK_FALSE(contact.isValid());
        }
    }

    SECTION(".localBlock()") {
        SECTION("// valid") {
            ContactReference contact = contactReference({ 1,2,2 }, Direction::plusX());
            CHECK(contact.localBlock() == b122);
        }

        SECTION("// invalid") {
            ContactReference contact = contactReference({ 0,2,2 }, Direction::plusX());
            CHECK_THROWS_AS(contact.localBlock(), std::out_of_range);
        }
    }

    SECTION(".maxPressureStress()") {
        SECTION("// valid") {
            ContactReference contact = contactReference({ 1,2,2 }, Direction::plusX());
            CHECK(contact.maxPressureStress() == PressureStress::minStress(concrete_20m, highTensile));
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.maxPressureStress(), std::out_of_range);
        }
    }

    SECTION(".normal()") {
        ContactReference contact = contactReference({ 2,2,2 }, Direction::minusX());
        CHECK(contact.normal() == -Normals::x);
    }

    SECTION(".opposite()") {
        SECTION("// valid") {
            ContactReference contact = contactReference({ 2,2,2 }, Direction::minusX());
            ContactReference expected = contactReference({ 1,2,2 }, Direction::plusX());
            CHECK(contact.opposite() == expected);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.opposite(), std::out_of_range);
        }
    }

    SECTION(".otherBlock()") {
        SECTION("// valid") {
            ContactReference contact = contactReference({ 1,2,2 }, Direction::plusX());
            CHECK(contact.otherBlock() == b222);
        }

        SECTION("// invalid") {
            ContactReference contact = contactReference({ 1,2,2 }, Direction::minusX());
            CHECK_THROWS_AS(contact.otherBlock(), std::out_of_range);
        }
    }

    SECTION(".solverIndex()") {
        auto const& structureData = *scene.blocks.find({ 7,7,7 }).structure();
        auto checkContact = [&](BlockIndex const& localBlockId, Direction direction) {
            ContactReference contact{ scene, ContactIndex{ localBlockId, direction } };
            auto const solverId = contact.solverIndex();
            auto const& link = structureData.solverStructure().links().at(solverId.linkIndex);
            std::optional<BlockIndex> otherBlockId = localBlockId.neighbourAlong(direction);
            auto const& solverIndices = structureData.solverIndices();
            REQUIRE(otherBlockId);
            if (solverId.isOnLocalNode) {
                CHECK(link.localNodeId() == solverIndices.at(localBlockId));
                CHECK(link.otherNodeId() == solverIndices.at(*otherBlockId));
            } else {
                CHECK(link.otherNodeId() == solverIndices.at(localBlockId));
                CHECK(link.localNodeId() == solverIndices.at(*otherBlockId));
            }
        };

        SECTION("// direction: plusX") {
            checkContact({ 6,7,7 }, Direction::plusX());
        }

        SECTION("// direction: plusY") {
            checkContact({ 7,6,7 }, Direction::plusY());
        }

        SECTION("// direction: plusZ") {
            checkContact({ 7,7,6 }, Direction::plusZ());
        }

        SECTION("// direction: minusX") {
            checkContact({ 7,7,7 }, Direction::minusX());
        }

        SECTION("// direction: minusY") {
            checkContact({ 7,7,7 }, Direction::minusY());
        }

        SECTION("// direction: minusZ") {
            checkContact({ 7,7,7 }, Direction::minusZ());
        }
    }

    SECTION(".structure()") {
        ContactReference c1 = contactReference({ 1,2,2 }, Direction::plusX());
        CHECK(c1.structure() == structureRefAt({ 1,2,2 }));
        ContactReference c2 = contactReference({ 2,2,2 }, Direction::minusX());
        CHECK(c2.structure() == structureRefAt({ 1,2,2 }));
    }

    SECTION(".thickness()") {
        ContactReference c = contactReference({ 7,7,7 }, Direction::minusX());
        CHECK(c.thickness() == blockSize.x());
    }
}
