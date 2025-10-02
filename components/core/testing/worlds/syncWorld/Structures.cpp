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
#include <gustave/core/worlds/syncWorld/Structures.hpp>

#include <TestHelpers.hpp>

using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::core::worlds::syncWorld::detail::WorldUpdater<libCfg>;
using Structures = gustave::core::worlds::syncWorld::Structures<libCfg>;

using BlockIndex = WorldData::Scene::BlockIndex;
using Solver = WorldData::Solver;
using StructureReference = Structures::StructureReference;
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

TEST_CASE("core::worlds::syncWorld::Structures") {
    WorldData world = makeWorld();
    Structures structures{ world };

    {
        Transaction t;
        t.addBlock({ {0,0,0}, concrete_20m, blockMass, false });
        t.addBlock({ {0,1,0}, concrete_20m, blockMass, true });
        WorldUpdater{ world }.runTransaction(t);
        t.clear();
        t.addBlock({ {0,2,0}, concrete_20m, blockMass, false });
        WorldUpdater{ world }.runTransaction(t);
    }

    auto structureOf = [&](BlockIndex const& index) {
        auto sceneStructures = world.scene.blocks().at(index).structures();
        REQUIRE(sceneStructures.size() == 1);
        return StructureReference{ world, sceneStructures[0].index() };
    };

    SECTION(".at()") {
        SECTION("// invalid") {
            CHECK_THROWS_AS(structures.at(10), std::out_of_range);
        }

        SECTION("// valid") {
            auto const s000 = structures.at(1);
            REQUIRE(s000.isValid());
            CHECK(s000.blocks().contains({ 0,0,0 }));
        }
    }

    SECTION(".begin() // & .end()") {
        std::vector<StructureReference> expected = { structureOf({0,0,0}), structureOf({0,2,0}) };
        CHECK_THAT(structures, matchers::c2::UnorderedRangeEquals(expected));
    }

    SECTION(".find()") {
        SECTION("// invalid") {
            auto const structure = structures.find(10);
            CHECK_FALSE(structure.isValid());
        }

        SECTION("// valid") {
            auto const s000 = structures.find(1);
            REQUIRE(s000.isValid());
            CHECK(s000.blocks().contains({ 0,0,0 }));
        }
    }

    SECTION(".size()") {
        CHECK(structures.size() == 2);
    }
}
