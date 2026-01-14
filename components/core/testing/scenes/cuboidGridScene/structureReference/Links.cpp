/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <array>

#include <gustave/core/scenes/cuboidGridScene/structureReference/Links.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <TestHelpers.hpp>
#include <SceneUserData.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

template<bool mut>
using Links = cuboid::structureReference::Links<libCfg, SceneUserData, mut>;

using BlockIndex = SceneData::BlockIndex;
using ContactIndex = Links<false>::ContactIndex;
using Direction = ContactIndex::Direction;
using StructureData = SceneData::StructureData;
using Transaction = SceneUpdater::Transaction;

TEST_CASE("core::scenes::cuboidGridScene::structureReference::Links") {
    auto const blockSize = vector3(1.f, 1.f, 1.f, u.length);
    auto scene = SceneData{ blockSize };

    Transaction t;
    t.addBlock({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,0,0}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,0,0}, concrete_20m, 3000.f * u.mass, false });

    t.addBlock({ {2,3,3}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,2,3}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,3,2}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,3,3}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {3,3,4}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {3,3,5}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,3,6}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {3,4,3}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {3,5,3}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,6,3}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {4,3,3}, concrete_20m, 2000.f * u.mass, false });
    t.addBlock({ {5,3,3}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {6,3,3}, concrete_20m, 2000.f * u.mass, false });
    SceneUpdater{ scene }.runTransaction(t);

    auto structDataOf = [&](BlockIndex const& index) -> StructureData& {
        return scene.structures.at(scene.blocks.at(index).structureId());
    };

    auto mLinks333 = Links<true>{ structDataOf({3,3,3}) };
    auto const& cmLinks333 = mLinks333;
    auto iLinks333 = Links<false>{ structDataOf({3,3,3}) };

    SECTION(".begin() // && .end()") {
        auto runTest = [&](auto&& links, bool expectedConst) {
            auto const expectedIds = std::array{
                ContactIndex{ {2,3,3}, Direction::plusX() },
                ContactIndex{ {3,2,3}, Direction::plusY() },
                ContactIndex{ {3,3,2}, Direction::plusZ() },
                ContactIndex{ {3,3,3}, Direction::plusX() },
                ContactIndex{ {3,3,3}, Direction::plusY() },
                ContactIndex{ {3,3,3}, Direction::plusZ() },
                ContactIndex{ {3,3,4}, Direction::plusZ() },
                ContactIndex{ {3,4,3}, Direction::plusY() },
                ContactIndex{ {4,3,3}, Direction::plusX() },
            };
            auto ids = mLinks333 | std::views::transform([](auto&& contactRef) { return contactRef.index(); });
            REQUIRE_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
            CHECK(expectedConst == (*links.begin()).structure().userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mLinks333, false);
        }

        SECTION("// const") {
            runTest(cmLinks333, true);
        }

        SECTION("// immutable") {
            runTest(iLinks333, true);
        }
    }
}
