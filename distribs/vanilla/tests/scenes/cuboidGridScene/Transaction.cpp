/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/scenes/cuboidGridScene/Transaction.hpp>

#include <TestHelpers.hpp>

namespace cuboid = gustave::scenes::cuboidGridScene;

using BlockConstructionInfo = cuboid::BlockConstructionInfo<libCfg>;
using BlockIndex = cuboid::BlockIndex;
using Transaction = cuboid::Transaction<libCfg>;

TEST_CASE("scenes::cuboidGridScene::Transaction") {
    Transaction transaction;
    auto const& newBlocks = transaction.newBlocks();
    auto const& deletedBlocks = transaction.deletedBlocks();

    SECTION(".addBlock(BlockConstructionInfo const&)") {
        BlockConstructionInfo const new1{ {2,4,6}, concrete_20m, 2.f * u.mass, true };
        BlockConstructionInfo const new2{ {2,4,7}, concrete_20m, 5.f * u.mass, true };
        transaction.addBlock(new1);
        transaction.addBlock(new2);

        BlockConstructionInfo const dup1{ {2,4,6}, concrete_20m, 8.f * u.mass, false };

        SECTION("// valid") {
            auto it = newBlocks.find(dup1);
            REQUIRE(it != newBlocks.end());
            CHECK(*it == new1);
        }

        SECTION("// duplicate") {
            CHECK_THROWS_AS(transaction.addBlock(dup1), std::invalid_argument);
        }
    }

    SECTION(".removeBlock(BlockIndex const&)") {
        transaction.removeBlock({ 1,1,5 });
        transaction.removeBlock({ 2,1,5 });

        CHECK(deletedBlocks.contains({ 1,1,5 }));
    }

    SECTION(".clear()") {
        transaction.addBlock({ {2,4,6}, concrete_20m, 2.f * u.mass, true });
        transaction.removeBlock({5,5,5});

        transaction.clear();

        CHECK(newBlocks.empty());
        CHECK(deletedBlocks.empty());
    }
}
