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

#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/testing/cPropPtr.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

namespace cuboidGrid = gustave::core::scenes::cuboidGridScene;

template<bool mut>
using BlockReference = cuboidGrid::BlockReference<libCfg, SceneUserData, mut>;
using SceneData = cuboidGrid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboidGrid::detail::SceneUpdater<libCfg, SceneUserData>;

using BlockIndex = BlockReference<false>::BlockIndex;
using ContactIndex = BlockReference<false>::ContactReference<false>::ContactIndex;
using Direction = ContactIndex::Direction;
using Transaction = SceneUpdater::Transaction;

static_assert(gustave::testing::cPropPtr<BlockReference<true>>);

TEST_CASE("core::scenes::cuboidGridScene::BlockReference") {
    auto const blockSize = vector3(2.f, 3.f, 1.f, u.length);
    SceneData sceneData{ blockSize };

    {
        Transaction t;
        t.addBlock({ {1,1,1}, concrete_20m, 1000.f * u.mass, false });
        t.addBlock({ {1,1,2}, concrete_20m, 9000.f * u.mass, true });
        t.addBlock({ {1,1,3}, concrete_20m, 2000.f * u.mass, false });
        SceneUpdater{ sceneData }.runTransaction(t);
    }

    auto deleteBlock = [&](BlockIndex const& blockId) {
        Transaction t;
        t.removeBlock(blockId);
        SceneUpdater{ sceneData }.runTransaction(t);
    };

    auto mb112 = BlockReference<true>{ sceneData, {1,1,2} };
    auto const& cmb112 = mb112;
    auto ib112 = BlockReference<false>{ sceneData, {1,1,2} };

    SECTION(".blockSize()") {
        CHECK(ib112.blockSize() == blockSize);
    }

    SECTION(".contacts()") {
        auto runTest = [&](auto&& blockRef, bool expectedConst) {
            auto contactRef = blockRef.contacts().along(Direction::minusZ());
            REQUIRE(contactRef.isValid());
            CHECK(contactRef.index() == ContactIndex{ blockRef.index(), Direction::minusZ()});
            CHECK(expectedConst == contactRef.structure().userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mb112, false);
        }

        SECTION("// const") {
            runTest(cmb112, true);
        }

        SECTION("// immutable") {
            runTest(ib112, true);
        }

        SECTION("// invalid") {
            deleteBlock(ib112.index());
            CHECK_THROWS_AS(ib112.contacts(), std::out_of_range);
        }
    }

    SECTION(".index()") {
        CHECK(ib112.index() == BlockIndex{ 1,1,2 });
    }

    SECTION(".isFoundation()") {
        SECTION("// valid") {
            CHECK(ib112.isFoundation());
        }

        SECTION("// invalid") {
            deleteBlock(ib112.index());
            CHECK_THROWS_AS(ib112.isFoundation(), std::out_of_range);
        }
    }

    SECTION(".isValid()") {
        REQUIRE(ib112.isValid());
        deleteBlock(ib112.index());
        REQUIRE_FALSE(ib112.isValid());
    }

    SECTION(".mass()") {
        SECTION("// valid") {
            CHECK(ib112.mass() == 9000.f * u.mass);
        }

        SECTION("// invalid") {
            deleteBlock(ib112.index());
            CHECK_THROWS_AS(ib112.mass(), std::out_of_range);
        }
    }

    SECTION(".maxPressureStress()") {
        SECTION("// valid") {
            CHECK(ib112.maxPressureStress() == concrete_20m);
        }

        SECTION("// invalid") {
            deleteBlock(ib112.index());
            CHECK_THROWS_AS(ib112.maxPressureStress(), std::out_of_range);
        }
    }

    SECTION(".position()") {
        CHECK(ib112.position() == vector3(2.f, 3.f, 2.f, u.length));
    }

    SECTION(".structures()") {
        auto runTest = [&](auto&& blockRef, bool expectedConst) {
            auto structs = blockRef.structures();
            REQUIRE(structs.size() == 2);
            auto& s0 = structs[0];
            CHECK(s0.blocks().contains(blockRef.index()));
            CHECK(expectedConst == s0.userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mb112, false);
        }

        SECTION("// const") {
            runTest(cmb112, true);
        }

        SECTION("// immutable") {
            runTest(ib112, true);
        }

        SECTION("// invalid") {
            deleteBlock(ib112.index());
            CHECK_THROWS_AS(ib112.structures(), std::out_of_range);
        }
    }
}
