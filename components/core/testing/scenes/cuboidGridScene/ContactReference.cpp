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
#include <gustave/testing/ConstDetector.hpp>
#include <gustave/testing/cPropPtr.hpp>

#include <TestHelpers.hpp>

namespace {
    struct UserData {
        using Structure = gustave::testing::ConstDetector<int>;
    };
}

namespace cuboid = gustave::core::scenes::cuboidGridScene;

template<bool mut>
using ContactReference = cuboid::ContactReference<libCfg, UserData, mut>;

using SceneData = cuboid::detail::SceneData<libCfg, UserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg,UserData>;

using BlockIndex = ContactReference<false>::BlockIndex;
using ContactIndex = ContactReference<false>::ContactIndex;
using Direction = ContactReference<false>::Direction;
using Transaction = SceneUpdater::Transaction;

template<bool mut>
using BlockReference = ContactReference<false>::BlockReference<mut>;

template<bool mut>
using StructureReference = ContactReference<false>::StructureReference<mut>;

static_assert(gustave::testing::cPropPtr<ContactReference<true>>);

TEST_CASE("core::scenes::cuboidGridScene::ContactReference") {
    auto const blockSize = vector3(2.f, 3.f, 1.f, u.length);
    SceneData scene{ blockSize };

    PressureStress highTensile{
        1'000'000.f * u.pressure,
        15'000'000.f * u.pressure,
        20'000'000.f * u.pressure,
    };

    Transaction t;
    t.addBlock({ {1,2,2}, highTensile, 1000.f * u.mass, false });
    t.addBlock({ {2,2,2}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {2,3,2}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {7,7,7}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {6,7,7}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {7,6,7}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {7,7,6}, concrete_20m, 1000.f * u.mass, false });
    SceneUpdater{ scene }.runTransaction(t);

    auto blockRef = [&](BlockIndex const& blockId) {
        auto result = BlockReference<false>{ scene, blockId };
        REQUIRE(result.isValid());
        return result;
    };

    auto structRef = [&](BlockIndex const& blockId) {
        auto const structId = scene.blocks.at(blockId).structureId();
        return StructureReference<false>{ scene.structures.atShared(structId) };
    };

    auto mutContactRef = [&](BlockIndex const& localBlockId, Direction direction) {
        return ContactReference<true>{ scene, { localBlockId, direction } };
    };

    auto immContactRef = [&](BlockIndex const& localBlockId, Direction direction) {
        return ContactReference<false>{ scene, { localBlockId, direction } };
    };

    auto mContact222minusX = mutContactRef({ 2,2,2 }, Direction::minusX());
    auto const& cmContact222minusX = mContact222minusX;
    auto iContact222minusX = immContactRef({ 2,2,2 }, Direction::minusX());

    auto invalidContact = immContactRef({ 0,0,0 }, Direction::minusZ());

    SECTION(".area()") {
        CHECK(iContact222minusX.area() == 3.f * u.area);
    }

    SECTION(".index()") {
        auto const expected = ContactIndex{ { 2,2,2 }, Direction::minusX() };
        CHECK(iContact222minusX.index() == expected);
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(iContact222minusX.isValid());
        }

        SECTION("// false: no local block") {
            auto const contact = immContactRef({ 2,1,2 }, Direction::plusY());
            CHECK_FALSE(contact.isValid());
        }

        SECTION("// false: no other block") {
            auto contact = immContactRef({ 2,2,2 }, Direction::minusY());
            CHECK_FALSE(contact.isValid());
        }

        SECTION("// false: both foundations") {
            auto contact = immContactRef({ 2,2,2 }, Direction::plusY());
            CHECK_FALSE(contact.isValid());
        }
    }

    SECTION(".localBlock()") {
        auto runValidTest = [&](auto&& contactRef, bool expectedConst) {
            auto res = contactRef.localBlock();
            auto const expected = blockRef({ 2,2,2 });
            CHECK(res == expected);
            CHECK(expectedConst == res.structures()[0].userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContact222minusX, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContact222minusX, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContact222minusX, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.localBlock(), std::out_of_range);
        }
    }

    SECTION(".maxPressureStress()") {
        SECTION("// valid") {
            CHECK(iContact222minusX.maxPressureStress() == PressureStress::minStress(concrete_20m, highTensile));
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.maxPressureStress(), std::out_of_range);
        }
    }

    SECTION(".normal()") {
        CHECK(iContact222minusX.normal() == -Normals::x);
    }

    SECTION(".opposite()") {
        auto runValidTest = [&](auto&& contactRef, bool expectedConst) {
            auto result = contactRef.opposite();
            auto const expected = immContactRef({ 1,2,2 }, Direction::plusX());
            CHECK(result == expected);
            CHECK(expectedConst == result.structure().userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContact222minusX, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContact222minusX, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContact222minusX, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.opposite(), std::out_of_range);
        }
    }

    SECTION(".otherBlock()") {
        auto runValidTest = [&](auto&& contactRef, bool expectedConst) {
            auto result = contactRef.otherBlock();
            auto const expected = blockRef({ 1,2,2 });
            CHECK(result == expected);
            CHECK(expectedConst == result.structures()[0].userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContact222minusX, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContact222minusX, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContact222minusX, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.otherBlock(), std::out_of_range);
        }
    }

    SECTION(".solverIndex()") {
        auto const structureId = scene.blocks.find({ 7,7,7 }).structureId();
        auto const& structureData = scene.structures.at(structureId);
        auto checkContact = [&](BlockIndex const& localBlockId, Direction direction) {
            auto const result = immContactRef(localBlockId, direction).solverIndex();
            auto const& solverLink = structureData.solverStructure().links().at(result.linkIndex);
            std::optional<BlockIndex> otherBlockId = localBlockId.neighbourAlong(direction);
            auto const& solverIndices = structureData.solverIndices();
            REQUIRE(otherBlockId);
            if (result.isOnLocalNode) {
                CHECK(solverLink.localNodeId() == solverIndices.at(localBlockId));
                CHECK(solverLink.otherNodeId() == solverIndices.at(*otherBlockId));
            } else {
                CHECK(solverLink.otherNodeId() == solverIndices.at(localBlockId));
                CHECK(solverLink.localNodeId() == solverIndices.at(*otherBlockId));
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

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.solverIndex(), std::out_of_range);
        }
    }

    SECTION(".structure()") {
        auto runValidTest = [&](auto&& contactRef, bool expectedConst) {
            auto result = contactRef.structure();
            auto const s122 = structRef({ 1,2,2 });
            auto const s777 = structRef({ 7,7,7 });
            CHECK(result == s122);
            CHECK(result != s777);
            CHECK(expectedConst == result.userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContact222minusX, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContact222minusX, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContact222minusX, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.structure(), std::out_of_range);
        }
    }

    SECTION(".thickness()") {
        CHECK(iContact222minusX.thickness() == blockSize.x());
    }
}
