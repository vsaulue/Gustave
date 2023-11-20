/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/Scene.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/testing/Matchers.hpp>

#include <TestConfig.hpp>

using BlockPosition = Gustave::Scenes::CuboidGrid::BlockPosition;
using Direction = Gustave::Math3d::BasicDirection;
using Scene = Gustave::Scenes::CuboidGrid::Scene<G::libConfig>;
using StructureData = Gustave::Scenes::CuboidGrid::detail::StructureData<G::libConfig>;
using Transaction = Gustave::Scenes::CuboidGrid::Transaction<G::libConfig>;

static constexpr G::Real<u.density> concreteDensity = 2'400.f * u.density;

TEST_CASE("Scene::CuboidGrid::Scene") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    G::Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;

    Scene scene{blockSize};

    SECTION("::blocks() const") {
        Scene::Blocks blocks = scene.blocks();

        SECTION("// empty") {
            CHECK(blocks.size() == 0);
            CHECK(blocks.begin() == blocks.end());
        }

        SECTION("// not empty") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {2,0,0}, concrete_20m, blockMass, false });
            scene.modify(t);

            CHECK(blocks.size() == 2);
            CHECK(blocks.at({1,0,0}).isFoundation());
            CHECK_FALSE(blocks.at({ 2,0,0 }).isFoundation());

            {
                std::vector<BlockPosition> positions;
                for (auto const& block : blocks) {
                    positions.push_back(block.position());
                }
                std::vector<BlockPosition> expected{ {1,0,0}, {2,0,0} };
                CHECK_THAT(positions, M::C2::UnorderedEquals(expected));
            }
        }
    }

    SECTION("::blockSize() const") {
        CHECK(scene.blockSize() == blockSize);
    }
}
