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

#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/Blocks.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>

#include <TestHelpers.hpp>

using BlockIndex = Gustave::Scenes::CuboidGrid::BlockIndex;
using BlockReference = Gustave::Scenes::CuboidGrid::BlockReference<cfg>;
using Blocks = Gustave::Scenes::CuboidGrid::Blocks<cfg>;
using SceneData = Gustave::Scenes::CuboidGrid::detail::SceneData<cfg>;

static_assert(std::ranges::forward_range<Blocks>);

TEST_CASE("Scenes::CuboidGrid::Blocks") {
    Vector3<u.length> const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneData sceneData{ blockSize };
    Blocks const blocks{ sceneData };

    SECTION(".at()") {
        sceneData.blocks.insert({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });

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
            sceneData.blocks.insert({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
            sceneData.blocks.insert({ {3,0,0}, concrete_20m, 1000.f * u.mass, false });

            std::vector<BlockIndex> indices;
            for (auto const& block : blocks) {
                indices.push_back(block.index());
            }
            std::vector<BlockIndex> const expected = { {1,0,0},{3,0,0} };
            CHECK_THAT(indices, M::C2::UnorderedEquals(expected));
        }
    }

    SECTION(".size()") {
        sceneData.blocks.insert({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
        CHECK(blocks.size() == 1);
    }
}
