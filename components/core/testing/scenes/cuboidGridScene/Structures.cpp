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

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/core/scenes/cuboidGridScene/Structures.hpp>

#include <TestHelpers.hpp>

namespace cuboid = ::gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg, void>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, void>;
using Structures = cuboid::Structures<libCfg, void>;
using Transaction = SceneUpdater::Transaction;

using BlockIndex = cuboid::BlockIndex;

static_assert(std::ranges::forward_range<Structures>);

TEST_CASE("core::scenes::cuboidGridScene::Structures") {
    Vector3<u.length> const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneData sceneData{ blockSize };
    Structures structures{ sceneData };

    Transaction t;
    t.addBlock({ {0,1,0}, concrete_20m, 1000.0f * u.mass, true });
    t.addBlock({ {0,2,0}, concrete_20m, 2000.0f * u.mass, false });
    t.addBlock({ {5,1,0}, concrete_20m, 51000.0f * u.mass, true });
    t.addBlock({ {5,2,0}, concrete_20m, 52000.0f * u.mass, false });
    SceneUpdater{ sceneData }.runTransaction(t);

    auto structureIdOf = [&](BlockIndex const& blockId) {
        auto const blockDataRef = sceneData.blocks.find(blockId);
        REQUIRE(blockDataRef);
        return blockDataRef.structureId();
    };

    SECTION(".at()") {
        SECTION("// valid") {
            auto const blockId = BlockIndex{ 0,2,0 };
            auto const structId = structureIdOf(blockId);
            auto const result = structures.at(structId);
            REQUIRE(result.isValid());
            REQUIRE(result.blocks().contains(blockId));
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(structures.at(10), std::out_of_range);
        }
    }

    SECTION(".begin() // & end()") {
        bool has1 = false;
        bool has5 = false;
        for (auto const& structRef : structures) {
            if (structRef.blocks().contains({ 0,1,0 })) {
                has1 = true;
                CHECK(structRef.blocks().contains({ 0,2,0 }));
            }
            if (structRef.blocks().contains({ 5,1,0 })) {
                has5 = true;
                CHECK(structRef.blocks().contains({ 5,2,0 }));
            }
        }
        CHECK((has1 && has5));
    }

    SECTION(".contains(StructReference const&)") {
        auto const structRef = *structures.begin();
        CHECK(structures.contains(structRef));

        t.clear();
        t.removeBlock({ 0,2,0 });
        t.removeBlock({ 5,2,0 });
        SceneUpdater{ sceneData }.runTransaction(t);

        CHECK_FALSE(structures.contains(structRef));
    }

    SECTION(".find()") {
        SECTION("// valid") {
            auto const blockId = BlockIndex{ 0,2,0 };
            auto const structId = structureIdOf(blockId);
            auto const result = structures.find(structId);
            REQUIRE(result.isValid());
            REQUIRE(result.blocks().contains(blockId));
        }

        SECTION("// invalid") {
            auto const structRef = structures.find(10);
            CHECK_FALSE(structRef.isValid());
        }
    }

    SECTION(".size()") {
        CHECK(structures.size() == 2);
    }
}
