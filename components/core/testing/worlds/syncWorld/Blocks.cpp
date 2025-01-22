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

#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldUpdater.hpp>
#include <gustave/core/worlds/syncWorld/Blocks.hpp>

#include <TestHelpers.hpp>

using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::core::worlds::syncWorld::detail::WorldUpdater<libCfg>;
using Blocks = gustave::core::worlds::syncWorld::Blocks<libCfg>;

using BlockIndex = WorldData::Scene::BlockIndex;
using BlockReference = Blocks::BlockReference;
using Solver = WorldData::Solver;
using Transaction = WorldUpdater::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ Solver::Config{ g, solverPrecision } };
    return WorldData{ blockSize, std::move(solver) };
}

TEST_CASE("core::worlds::syncWorld::Blocks") {
    WorldData world = makeWorld();
    Blocks blocks{ world };

    {
        Transaction t;
        t.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
        t.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,3,0}, concrete_20m, blockMass, false });
        WorldUpdater{ world }.runTransaction(t);
    }

    SECTION(".at()") {
        SECTION("// valid") {
            auto const block = blocks.at({ 0,0,0 });
            REQUIRE(block.isValid());
            CHECK(block.isFoundation());
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(blocks.at({ 1,0,0 }), std::out_of_range);
        }
    }

    SECTION(".begin() // & .end()") {
        std::vector<BlockIndex> indices;
        for (auto const& block : blocks) {
            indices.push_back(block.index());
        }
        std::vector<BlockIndex> expected{ {0,0,0},{0,1,0},{0,2,0},{0,3,0} };
        CHECK_THAT(indices, matchers::c2::UnorderedRangeEquals(expected));
    }

    SECTION(".find()") {
        CHECK(blocks.find({ 0,0,0 }).isValid());
        CHECK_FALSE(blocks.find({ 0,0,1 }).isValid());
    }

    SECTION(".size()") {
        CHECK(blocks.size() == 4);
    }
}
