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

#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>

#include <TestHelpers.hpp>

namespace cuboidGrid = gustave::core::scenes::cuboidGridScene;

using BlockReference = cuboidGrid::BlockReference<libCfg>;
using SceneData = cuboidGrid::detail::SceneData<libCfg>;
using SceneUpdater = cuboidGrid::detail::SceneUpdater<libCfg>;

using BlockIndex = BlockReference::BlockIndex;
using ContactIndex = BlockReference::ContactReference::ContactIndex;
using ContactReference = BlockReference::ContactReference;
using Direction = BlockReference::Direction;
using StructureReference = BlockReference::StructureReference;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<BlockReference::Contacts>);
static_assert(std::ranges::forward_range<BlockReference::Structures>);

TEST_CASE("core::scenes::cuboidGridScene::BlockReference") {
    auto const blockSize = vector3(2.f, 3.f, 1.f, u.length);
    SceneData sceneData{ blockSize };

    Transaction t;
    auto newBlock = [&](BlockIndex const& index, Real<u.mass> mass, bool isFoundation) -> BlockReference {
        t.addBlock({ index, concrete_20m, mass, isFoundation });
        return BlockReference{ sceneData, index };
    };
    BlockReference b000 = newBlock({ 0,0,0 }, 1000.f * u.mass, true);
    BlockReference b111 = newBlock({ 1,1,1 }, 3000.f * u.mass, false);
    /* b011 */ newBlock({0,1,1}, 4000.f * u.mass, false);
    BlockReference b211 = newBlock({ 2,1,1 }, 5000.f * u.mass, false);
    BlockReference b101 = newBlock({ 1,0,1 }, 6000.f * u.mass, false);
    BlockReference b121 = newBlock({ 1,2,1 }, 7000.f * u.mass, true);
    /* b110 */ newBlock({1,1,0}, 8000.f * u.mass, false);
    BlockReference b112 = newBlock({ 1,1,2 }, 9000.f * u.mass, true);
    BlockReference b122 = newBlock({1,2,2}, 2000.f * u.mass, false);
    /* b113 */ newBlock({1,1,3}, 1000.f * u.mass, true);
    BlockReference b777 = newBlock({ 7,7,7 }, 1000.f * u.mass, true);
    BlockReference b778 = newBlock({ 7,7,8 }, 1000.f * u.mass, false);
    /* b787 */ newBlock({7,8,7}, 1000.f * u.mass, false);
    /* b788 */ newBlock({7,8,8}, 1000.f * u.mass, false);
    SceneUpdater{ sceneData }.runTransaction(t);

    SECTION(".blockSize()") {
        CHECK(b101.blockSize() == blockSize);
    }

    SECTION(".contacts()") {
        auto makeContactRef = [&](BlockReference const& source, Direction direction) {
            return ContactReference{ sceneData, ContactIndex{ source.index(), direction}};
        };

        SECTION(".along()") {
            SECTION("// valid") {
                ContactReference contact = b121.contacts().along(Direction::minusY());
                CHECK(contact == makeContactRef(b121, Direction::minusY()));
            }

            SECTION("// invalid") {
                CHECK_THROWS_AS(b121.contacts().along(Direction::plusY()), std::out_of_range);
            }
        }

        SECTION(".begin() // & .end()") {
            SECTION("// empty") {
                auto contacts = b000.contacts();
                CHECK(contacts.begin() == contacts.end());
            }

            SECTION("// 6 contacts") {
                std::vector<ContactReference> expected = {
                    makeContactRef(b111, Direction::minusX()),
                    makeContactRef(b111, Direction::plusX()),
                    makeContactRef(b111, Direction::minusY()),
                    makeContactRef(b111, Direction::plusY()),
                    makeContactRef(b111, Direction::minusZ()),
                    makeContactRef(b111, Direction::plusZ()),
                };
                CHECK_THAT(b111.contacts(), matchers::c2::UnorderedRangeEquals(expected));
            }
        }
    }

    SECTION(".index()") {
        CHECK(b121.index() == BlockIndex{ 1,2,1 });
    }

    SECTION(".isFoundation()") {
        SECTION("// valid") {
            CHECK_FALSE(b111.isFoundation());
        }

        SECTION("// invalid") {
            sceneData.blocks.erase({ 1,1,1 });
            CHECK_THROWS_AS(b111.isFoundation() == false, std::out_of_range);
        }
    }

    SECTION(".isValid()") {
        REQUIRE(b111.isValid());
        sceneData.blocks.erase(b111.index());
        REQUIRE_FALSE(b111.isValid());
    }

    SECTION(".mass()") {
        SECTION("// valid") {
            CHECK(b111.mass() == 3000.f * u.mass);
        }
        SECTION("// invalid") {
            sceneData.blocks.erase({ 1,1,1 });
            CHECK_THROWS_AS(b111.mass() == 0.f * u.mass, std::out_of_range);
        }
    }

    SECTION(".maxPressureStress()") {
        SECTION("// valid") {
            CHECK(b111.maxPressureStress() == concrete_20m);
        }

        SECTION("// invalid") {
            sceneData.blocks.erase({ 1,1,1 });
            CHECK_THROWS_AS(b111.maxPressureStress() == concrete_20m, std::out_of_range);
        }
    }

    SECTION(".position()") {
        CHECK(b211.position() == vector3(4.f, 3.f, 1.f, u.length));
    }

    SECTION(".structures()") {
        auto structureOf = [&](BlockReference const& block) {
            auto const blockDataRef = sceneData.blocks.find(block.index());
            REQUIRE(blockDataRef);
            auto const structureDataPtr = blockDataRef.structure();
            REQUIRE(structureDataPtr != nullptr);
            auto iterator = sceneData.structures.find(structureDataPtr);
            REQUIRE(iterator != sceneData.structures.end());
            return StructureReference{ *iterator };
        };
        SECTION("// non-foundation") {
            auto const structures = b111.structures();
            REQUIRE(structures.size() == 1);
            std::vector<StructureReference> expected = {
                structureOf(b111),
            };
            CHECK_THAT(structures, matchers::c2::UnorderedRangeEquals(expected));
        }

        SECTION("// foundation") {
            auto const structures = b112.structures();
            REQUIRE(structures.size() == 2);
            std::vector<StructureReference> expected = {
                structureOf(b111),
                structureOf(b122),
            };
            CHECK_THAT(structures, matchers::c2::UnorderedRangeEquals(expected));
        }

        SECTION("// foundation, ensures duplicates are filtered") {
            auto const structures = b777.structures();
            REQUIRE(structures.size() == 1);
            std::vector<StructureReference> expected = {
                structureOf(b778),
            };
            CHECK_THAT(structures, matchers::c2::UnorderedRangeEquals(expected));
        }
    }
}
