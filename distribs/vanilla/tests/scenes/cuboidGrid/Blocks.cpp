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

#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/Blocks.hpp>

#include <TestConfig.hpp>

using BlockPosition = Gustave::Scenes::CuboidGrid::BlockPosition;
using BlockReference = Gustave::Scenes::CuboidGrid::BlockReference<G::libConfig>;
using Blocks = Gustave::Scenes::CuboidGrid::Blocks<G::libConfig>;
using SceneBlocks = Gustave::Scenes::CuboidGrid::detail::SceneBlocks<G::libConfig>;

static_assert(std::ranges::forward_range<Blocks>);

TEST_CASE("Scenes::CuboidGrid::Blocks") {
    G::Vector3<u.length> const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneBlocks blocksData{ blockSize };
    Blocks const blocks{ blocksData };

    SECTION(".at()") {
        blocksData.insert({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });

        SECTION("// valid") {
            BlockReference ref = blocks.at({ 1,0,0 });
            CHECK(ref.mass() == 1000.f * u.mass);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(blocks.at({ 0,0,8 }), std::out_of_range);
        }
    }

    SECTION(".find()") {
        BlockReference ref = blocks.find({ 3,2,1 });
        CHECK_FALSE(ref.isValid());
    }

    SECTION(".begin() // & .end()") {
        SECTION(": empty") {
            CHECK(blocks.begin() == blocks.end());
        }

        SECTION(": non-empty") {
            blocksData.insert({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
            blocksData.insert({ {3,0,0}, concrete_20m, 1000.f * u.mass, false });

            std::vector<BlockPosition> positions;
            for (auto const& block : blocks) {
                positions.push_back(block.position());
            }
            std::vector<BlockPosition> const expected = { {1,0,0},{3,0,0} };
            CHECK_THAT(positions, M::C2::UnorderedEquals(expected));
        }
    }

    SECTION(".size()") {
        blocksData.insert({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
        CHECK(blocks.size() == 1);
    }
}
