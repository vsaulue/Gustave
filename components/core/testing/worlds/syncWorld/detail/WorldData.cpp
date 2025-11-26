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

#include <TestHelpers.hpp>

using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;

using Solver = WorldData::Solver;

TEST_CASE("core::worlds::syncWorld::detail::WorldData") {
    auto const blockSize = vector3(3.f, 2.f, 1.f, u.length);
    auto const solver = Solver{ Solver::Config{ g, 0.001f } };

    auto world1 = WorldData{ blockSize, solver };

    {
        WorldData::Scene::Transaction t;
        t.addBlock({ {1,1,1}, concrete_20m, 1000.f * u.mass, false });
        world1.scene.modify(t);
    }

    auto b1 = world1.scene.blocks().at({ 1,1,1 });
    b1.structures().unique().userData().init(world1);

    SECTION("// move") {
        auto checkMovedWorld = [&](WorldData const& movedWorld) {
            CHECK(std::ranges::all_of(movedWorld.scene.structures(), [&](auto&& structRef) {
                return &structRef.userData().world() == &movedWorld;
            }));
            CHECK(movedWorld.scene.blocks().size() == 1);
            CHECK(movedWorld.solver.config().targetMaxError() == solver.config().targetMaxError());
        };

        SECTION("// assign") {
            auto world2 = WorldData{ std::move(world1) };

            checkMovedWorld(world2);
        }

        SECTION("// constructor") {
            auto world2 = WorldData{ blockSize, Solver{ Solver::Config{ g, 0.1f } } };
            world2 = std::move(world1);

            checkMovedWorld(world2);
        }
    }
}
