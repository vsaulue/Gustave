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
#include <gustave/testing/ConstDetector.hpp>
#include <gustave/testing/cPropPtr.hpp>

#include <TestHelpers.hpp>

struct UserData {
    using Structure = gustave::testing::ConstDetector<int>;
};

template<bool isMutable>
using StructureReference = gustave::core::scenes::cuboidGridScene::StructureReference<libCfg, UserData, isMutable>;
using SceneData = gustave::core::scenes::cuboidGridScene::detail::SceneData<libCfg, UserData>;
using SceneUpdater = gustave::core::scenes::cuboidGridScene::detail::SceneUpdater<libCfg, UserData>;

using BlockIndex = StructureReference<true>::BlockIndex;
using BlockReference = StructureReference<true>::BlockReference;
using ContactIndex = StructureReference<true>::ContactIndex;
using ContactReference = StructureReference<true>::ContactReference;
using Direction = StructureReference<true>::ContactIndex::Direction;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<StructureReference<true>::Links>);

static_assert(gustave::testing::cPropPtr<StructureReference<true>>);

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

    auto structureDataOf = [&](BlockIndex const& blockId) {
        auto const blockDataRef = data.blocks.find(blockId);
        REQUIRE(blockDataRef);
        auto const structId = blockDataRef.structureId();
        REQUIRE(structId != data.structureIdGenerator.invalidIndex());
        return data.structures.atShared(structId);
    };

    auto makeContactRef = [&](BlockIndex const& source, Direction direction) {
        return ContactReference{ data, ContactIndex{ source, direction } };
    };

    auto ms1 = StructureReference<true>{ structureDataOf({ 1,0,0 }) };
    auto is1 = StructureReference<false>{ structureDataOf({ 1,0,0 }) };

    auto ms3 = StructureReference<true>{ structureDataOf({ 3,0,0 }) };
    auto const& cms3 = ms3;
    auto is3 = StructureReference<false>{ structureDataOf({ 3,0,0 }) };

    auto s666 = StructureReference<false>{ structureDataOf({ 6,6,6 }) };
    auto sInvalid = StructureReference<true>{ gustave::utils::NO_INIT };

    SECTION(".blocks()") {
        auto runTest = [&](auto&& structRef, bool expectedConst) {
            auto blocks = structRef.blocks();
            auto bRef = blocks.at({ 3,0,0 });
            REQUIRE(bRef.isValid());
            CHECK(bRef.mass() == 3000.f * u.mass);
            CHECK(expectedConst == bRef.structures()[0].userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(ms3, false);
        }

        SECTION("// const") {
            runTest(cms3, true);
        }

        SECTION("// immutable") {
            runTest(is3, true);
        }
    }

    SECTION(".contacts()") {
        SECTION(".at()") {
            SECTION("// valid") {
                ContactReference contact = is1.contacts().at(ContactIndex{ {1,0,0}, Direction::plusX() });
                CHECK(contact == makeContactRef({ 1,0,0 }, Direction::plusX()));
            }

            SECTION("// invalid source") {
                CHECK_THROWS_AS(is1.contacts().at(ContactIndex{ {0,0,0}, Direction::plusX() }), std::out_of_range);
            }

            SECTION("// invalid other") {
                CHECK_THROWS_AS(is1.contacts().at(ContactIndex{ {1,0,0}, Direction::plusY() }), std::out_of_range);
            }

            SECTION("// invalid structure") {
                CHECK_THROWS_AS(is3.contacts().at(ContactIndex{ {1,0,0}, Direction::plusX() }), std::out_of_range);
            }
        }
    }

    SECTION(".index()") {
        SECTION("// valid") {
            auto const res = is1.index();
            CHECK(res != data.structureIdGenerator.invalidIndex());
        }

        SECTION("// invalidated") {
            auto const expected = is1.index();
            Transaction t;
            t.removeBlock({ 1,0,0 });
            SceneUpdater{ data }.runTransaction(t);
            CHECK(is1.index() == expected);
        }

        SECTION("// invalid index") {
            auto const invalidStructure = StructureReference<true>{ gustave::utils::NO_INIT };
            CHECK_THROWS_AS(invalidStructure.index(), std::out_of_range);
        }
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(is1.isValid());
        }

        SECTION("// false") {
            Transaction t;
            t.removeBlock({ 1,0,0 });
            SceneUpdater{ data }.runTransaction(t);
            CHECK_FALSE(is1.isValid());
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
            auto opt = is3.solverIndexOf({ 1,0,0 });
            CHECK_FALSE(opt);
        }

        SECTION("// & .solverStructurePtr()") {
            auto const& solver3 = *is3.solverStructurePtr();
            auto const optIndex2 = is3.solverIndexOf({ 2,0,0 });
            auto const optIndex3 = is3.solverIndexOf({ 3,0,0 });
            auto const optIndex4 = is3.solverIndexOf({ 4,0,0 });
            REQUIRE(optIndex2);
            REQUIRE(optIndex3);
            REQUIRE(optIndex4);
            CHECK(solver3.nodes()[*optIndex2].mass() == 2000.f * u.mass);
            CHECK(solver3.nodes()[*optIndex3].mass() == 3000.f * u.mass);
            CHECK(solver3.nodes()[*optIndex4].mass() == 4000.f * u.mass);
        }
    }

    SECTION(".userData()") {
        SECTION("// invalid - mutable") {
            CHECK_THROWS_AS(sInvalid.userData(), std::out_of_range);
        }

        SECTION("// invalid - const") {
            auto const& asConst = sInvalid;
            CHECK_THROWS_AS(asConst.userData(), std::out_of_range);
        }

        SECTION("// invalid - immutable") {
            auto asImmutable = sInvalid.asImmutable();
            CHECK_THROWS_AS(asImmutable.userData(), std::out_of_range);
        }

        SECTION("// valid") {
            CHECK_FALSE(ms3.userData().isCalledAsConst());
            CHECK(cms3.userData().isCalledAsConst());
            CHECK(is3.userData().isCalledAsConst());
            ms3.userData().tag = 5;
            CHECK(is3.userData().tag == 5);
            CHECK(is1.userData().tag == 0);
        }
    }

    SECTION(".operator==()") {
        SECTION("// mutable == immutable") {
            CHECK(is3 == cms3);
            CHECK(cms3 == is3);
            CHECK(is1 != cms3);
            CHECK(ms1 != is3);
            CHECK(is3 != sInvalid);
        }

        SECTION("// immutable == immutable") {
            CHECK(is1 == is1);
            CHECK(is3 != is1);
        }

        SECTION("// mutable == mutable") {
            CHECK(ms3 == ms3);
            CHECK(ms1 != ms3);
        }
    }
}
