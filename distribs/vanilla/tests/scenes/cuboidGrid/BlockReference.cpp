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

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/StructureReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneUpdater.hpp>

#include <TestHelpers.hpp>

namespace CuboidGrid = Gustave::Scenes::CuboidGrid;

using BlockPosition = CuboidGrid::BlockPosition;
using BlockReference = CuboidGrid::BlockReference<cfg>;
using Direction = Gustave::Math3d::BasicDirection;
using Neighbour = BlockReference::Neighbour;
using SceneData = CuboidGrid::detail::SceneData<cfg>;
using SceneUpdater = CuboidGrid::detail::SceneUpdater<cfg>;
using StructureReference = CuboidGrid::StructureReference<cfg>;
using Transaction = CuboidGrid::Transaction<cfg>;

static_assert(std::ranges::forward_range<BlockReference::Neighbours>);
static_assert(std::ranges::forward_range<BlockReference::Structures>);

TEST_CASE("Scene::CuboidGrid::BlockReference") {
    auto const blockSize = vector3(2.f, 3.f, 1.f, u.length);
    SceneData sceneData{ blockSize };

    Transaction t;
    auto newBlock = [&](BlockPosition const& position, Real<u.mass> mass, bool isFoundation) -> BlockReference {
        t.addBlock({ position, concrete_20m, mass, isFoundation });
        return BlockReference{ sceneData, position };
    };
    BlockReference b000 = newBlock({ 0,0,0 }, 1000.f * u.mass, true);
    BlockReference b111 = newBlock({ 1,1,1 }, 3000.f * u.mass, false);
    BlockReference b011 = newBlock({ 0,1,1 }, 4000.f * u.mass, false);
    BlockReference b211 = newBlock({ 2,1,1 }, 5000.f * u.mass, false);
    BlockReference b101 = newBlock({ 1,0,1 }, 6000.f * u.mass, false);
    BlockReference b121 = newBlock({ 1,2,1 }, 7000.f * u.mass, true);
    BlockReference b110 = newBlock({ 1,1,0 }, 8000.f * u.mass, false);
    BlockReference b112 = newBlock({ 1,1,2 }, 9000.f * u.mass, true);
    /* b112 */ newBlock({1,2,2}, 2000.f * u.mass, false);
    /* b113 */ newBlock({1,1,3}, 1000.f * u.mass, true);
    SceneUpdater{ sceneData }.runTransaction(t);

    SECTION(".blockSize()") {
        CHECK(b101.blockSize() == blockSize);
    }

    SECTION(".mass()") {
        SECTION("// valid") {
            CHECK(b111.mass() == 3000.f * u.mass);
        }
        SECTION("// invalid") {
            sceneData.blocks.erase({ 1,1,1 });
            CHECK_THROWS_AS(b111.mass() == 0.f * u.mass, std::out_of_range);
        }
    }

    SECTION(".maxStress()") {
        SECTION("// valid") {
            CHECK(b111.maxStress() == concrete_20m);
        }

        SECTION("// invalid") {
            sceneData.blocks.erase({ 1,1,1 });
            CHECK_THROWS_AS(b111.maxStress() == concrete_20m, std::out_of_range);
        }
    }

    SECTION(".isFoundation()") {
        SECTION("// valid") {
            CHECK_FALSE(b111.isFoundation());
        }

        SECTION("// invalid") {
            sceneData.blocks.erase({ 1,1,1 });
            CHECK_THROWS_AS(b111.isFoundation() == false, std::out_of_range);
        }
    }

    SECTION(".isValid()") {
        REQUIRE(b111.isValid());
        sceneData.blocks.erase(b111.position());
        REQUIRE_FALSE(b111.isValid());
    }

    SECTION(".neighbours()") {
        auto neighboursAsVector = [](BlockReference const& block) -> std::vector<Neighbour> {
            std::vector<Neighbour> result;
            for (auto const& neighbour : block.neighbours()) {
                result.push_back(neighbour);
            }
            return result;
            };

        SECTION("// 6 neighbours") {
            auto const neighbours = neighboursAsVector(b111);
            auto const expected = std::vector<Neighbour>{
                Neighbour{ b011, Direction::minusX },
                Neighbour{ b211, Direction::plusX },
                Neighbour{ b101, Direction::minusY },
                Neighbour{ b121, Direction::plusY },
                Neighbour{ b110, Direction::minusZ },
                Neighbour{ b112, Direction::plusZ },
            };
            CHECK_THAT(neighbours, M::C2::UnorderedEquals(expected));
        }

        SECTION("// 1 neighbour") {
            auto const neighbours = neighboursAsVector(b011);
            auto const expected = std::vector<Neighbour>{
                Neighbour{ b111, Direction::plusX },
            };
            CHECK_THAT(neighbours, M::C2::UnorderedEquals(expected));
        }

        SECTION("// 0 neighbour") {
            auto const neighbours = neighboursAsVector(b000);
            CHECK(neighbours.size() == 0);
        }
    }

    SECTION(".position()") {
        CHECK(b121.position() == BlockPosition{ 1,2,1 });
    }

    SECTION(".structures()") {
        SECTION("// non-foundation") {
            auto const structures = b111.structures();
            REQUIRE(structures.size() == 1);
            CHECK(structures[0].blocks().contains(b111.position()));
        }

        SECTION("// foundation") {
            auto const structures = b112.structures();
            REQUIRE(structures.size() == 2);
            CHECK(structures[0] != structures[1]);
            bool has111 = false;
            bool has122 = false;
            for (auto const& structure : structures) {
                has111 = has111 || structure.blocks().contains({ 1,1,1 });
                has122 = has122 || structure.blocks().contains({ 1,2,2 });
            }
            CHECK((has111 && has122));
        }
    }
}
