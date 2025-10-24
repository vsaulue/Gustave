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
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>

#include <TestHelpers.hpp>

using StructureReference = gustave::core::scenes::cuboidGridScene::StructureReference<libCfg, void>;
using SceneData = gustave::core::scenes::cuboidGridScene::detail::SceneData<libCfg, void>;
using SceneUpdater = gustave::core::scenes::cuboidGridScene::detail::SceneUpdater<libCfg, void>;

using BlockIndex = StructureReference::BlockIndex;
using BlockReference = StructureReference::BlockReference;
using ContactIndex = StructureReference::ContactIndex;
using ContactReference = StructureReference::ContactReference;
using Direction = StructureReference::ContactIndex::Direction;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<StructureReference::Blocks>);
static_assert(std::ranges::forward_range<StructureReference::Links>);

TEST_CASE("core::scenes::cuboidGridScene::StructureReference") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneData data{ blockSize };

    {
        Transaction t;
        t.addBlock({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
        t.addBlock({ {2,0,0}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {3,0,0}, concrete_20m, 3000.f * u.mass, false });
        t.addBlock({ {4,0,0}, concrete_20m, 4000.f * u.mass, false });

        t.addBlock({ {5,6,6}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {6,5,6}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {6,6,5}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {6,6,6}, concrete_20m, 2000.f * u.mass, false });
        t.addBlock({ {6,6,7}, concrete_20m, 2000.f * u.mass, false });
        t.addBlock({ {6,6,8}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {6,6,9}, concrete_20m, 2000.f * u.mass, false });
        t.addBlock({ {6,7,6}, concrete_20m, 2000.f * u.mass, false });
        t.addBlock({ {6,8,6}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {6,9,6}, concrete_20m, 2000.f * u.mass, false });
        t.addBlock({ {7,6,6}, concrete_20m, 2000.f * u.mass, false });
        t.addBlock({ {8,6,6}, concrete_20m, 2000.f * u.mass, true });
        t.addBlock({ {9,6,6}, concrete_20m, 2000.f * u.mass, false });
        SceneUpdater{ data }.runTransaction(t);
    }

    auto structureOf = [&](BlockIndex const& index) -> StructureReference {
        auto const blockDataRef = data.blocks.find(index);
        REQUIRE(blockDataRef);
        auto const structId = blockDataRef.structureId();
        REQUIRE(structId != data.structureIdGenerator.invalidIndex());
        auto const it = data.structures.find(structId);
        REQUIRE(it != data.structures.end());
        return StructureReference{ *it };
    };

    auto makeContactRef = [&](BlockIndex const& source, Direction direction) {
        return ContactReference{ data, ContactIndex{ source, direction } };
    };

    auto const s1 = structureOf({ 1,0,0 });
    auto const s3 = structureOf({ 3,0,0 });
    auto const s666 = structureOf({ 6,6,6, });

    SECTION(".blocks()") {
        SECTION(".at()") {
            SECTION("// valid") {
                BlockReference r4 = s3.blocks().at({ 4,0,0 });
                CHECK(r4.mass() == 4000.f * u.mass);
            }

            SECTION("// invalid") {
                auto const blocks = s3.blocks();
                CHECK_THROWS_AS(blocks.at({ 1,0,0 }), std::out_of_range);
            }
        }

        SECTION(".begin() // & end()") {
            std::vector<BlockIndex> blocks;
            for (auto const& block : s1.blocks()) {
                blocks.push_back(block.index());
            }
            std::vector<BlockIndex> expected = { {1,0,0},{2,0,0} };
            CHECK_THAT(blocks, matchers::c2::UnorderedRangeEquals(expected));
        }

        SECTION(".contains()") {
            SECTION("// true") {
                CHECK(s1.blocks().contains({ 2,0,0 }));
            }

            SECTION("// false") {
                CHECK_FALSE(s3.blocks().contains({ 1,0,0 }));
            }
        }

        SECTION(".find()") {
            SECTION("// valid") {
                auto optBlock = s3.blocks().find({ 2,0,0 });
                REQUIRE(optBlock);
                CHECK(optBlock->isFoundation());
            }

            SECTION("// invalid") {
                auto optBlock = s3.blocks().find({ 1,0,0 });
                CHECK_FALSE(optBlock);
            }
        }

        SECTION(".size()") {
            CHECK(s3.blocks().size() == 3);
        }
    }

    SECTION(".contacts()") {
        SECTION(".at()") {
            SECTION("// valid") {
                ContactReference contact = s1.contacts().at(ContactIndex{ {1,0,0}, Direction::plusX() });
                CHECK(contact == makeContactRef({ 1,0,0 }, Direction::plusX()));
            }

            SECTION("// invalid source") {
                CHECK_THROWS_AS(s1.contacts().at(ContactIndex{ {0,0,0}, Direction::plusX() }), std::out_of_range);
            }

            SECTION("// invalid other") {
                CHECK_THROWS_AS(s1.contacts().at(ContactIndex{ {1,0,0}, Direction::plusY() }), std::out_of_range);
            }

            SECTION("// invalid structure") {
                CHECK_THROWS_AS(s3.contacts().at(ContactIndex{ {1,0,0}, Direction::plusX() }), std::out_of_range);
            }
        }
    }

    SECTION(".index()") {
        SECTION("// valid") {
            auto const res = s1.index();
            CHECK(res != data.structureIdGenerator.invalidIndex());
        }

        SECTION("// invalidated") {
            auto const expected = s1.index();
            Transaction t;
            t.removeBlock({ 1,0,0 });
            SceneUpdater{ data }.runTransaction(t);
            CHECK(s1.index() == expected);
        }

        SECTION("// invalid index") {
            auto const invalidStructure = StructureReference{ gustave::utils::NO_INIT };
            CHECK_THROWS_AS(invalidStructure.index(), std::out_of_range);
        }
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(s1.isValid());
        }

        SECTION("// false") {
            Transaction t;
            t.removeBlock({ 1,0,0 });
            SceneUpdater{ data }.runTransaction(t);
            CHECK_FALSE(s1.isValid());
        }
    }

    SECTION(".links()") {
        SECTION(".begin() // && .end()") {
            std::vector<ContactReference> expected = {
                makeContactRef({5,6,6}, Direction::plusX()),
                makeContactRef({6,5,6}, Direction::plusY()),
                makeContactRef({6,6,5}, Direction::plusZ()),
                makeContactRef({6,6,6}, Direction::plusX()),
                makeContactRef({6,6,6}, Direction::plusY()),
                makeContactRef({6,6,6}, Direction::plusZ()),
                makeContactRef({6,6,7}, Direction::plusZ()),
                makeContactRef({6,7,6}, Direction::plusY()),
                makeContactRef({7,6,6}, Direction::plusX()),
            };
            CHECK_THAT(s666.links(), matchers::c2::UnorderedRangeEquals(expected));
        }
    }

    SECTION(".solverIndexOf() ") {
        SECTION("// invalid") {
            auto opt = s3.solverIndexOf({ 1,0,0 });
            CHECK_FALSE(opt);
        }

        SECTION("// & .solverStructurePtr()") {
            auto const& solver3 = *s3.solverStructurePtr();
            auto const optIndex2 = s3.solverIndexOf({ 2,0,0 });
            auto const optIndex3 = s3.solverIndexOf({ 3,0,0 });
            auto const optIndex4 = s3.solverIndexOf({ 4,0,0 });
            REQUIRE(optIndex2);
            REQUIRE(optIndex3);
            REQUIRE(optIndex4);
            CHECK(solver3.nodes()[*optIndex2].mass() == 2000.f * u.mass);
            CHECK(solver3.nodes()[*optIndex3].mass() == 3000.f * u.mass);
            CHECK(solver3.nodes()[*optIndex4].mass() == 4000.f * u.mass);
        }
    }
}
