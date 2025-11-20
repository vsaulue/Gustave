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

#include <gustave/core/scenes/cuboidGridScene/detail/SceneStructures.hpp>
#include <gustave/utils/prop/SharedPtr.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

#include <initializer_list>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneStructures = cuboid::detail::SceneStructures<libCfg, SceneUserData>;

using StructureData = SceneStructures::StructureData;
using SceneData = StructureData::SceneData;

using BlockConstructionInfo = SceneData::Blocks::BlockConstructionInfo;
using BlockIndex = SceneData::Blocks::BlockIndex;
using StructureIndex = StructureData::StructureIndex;

template<typename T>
using SharedPtr = gustave::utils::prop::SharedPtr<T>;

static_assert(std::ranges::forward_range<SceneStructures>);

TEST_CASE("core::scenes::cuboidGridScene::detail::SceneStructures") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    SceneData scene{ blockSize };

    auto const blockInfos = std::initializer_list<BlockConstructionInfo>{
        { {0,0,0}, concrete_20m, 1000.f * u.mass, true },
        { {0,1,0}, concrete_20m, 1000.f * u.mass, false },
        { {0,2,0}, concrete_20m, 1000.f * u.mass, false },
        { {0,0,5}, concrete_20m, 1000.f * u.mass, true },
        { {0,1,5}, concrete_20m, 1000.f * u.mass, false },
        { {6,0,0}, concrete_20m, 1000.f * u.mass, false },
    };
    for (auto const& info : blockInfos) {
        scene.blocks.insert(info);
    }

    auto makeStructure = [&](StructureIndex id, BlockIndex const& root) {
        return SharedPtr<StructureData>{ std::make_shared<StructureData>(id, scene, scene.blocks.at(root)) };
    };

    auto s010 = makeStructure(0, { 0,1,0 });
    auto s015 = makeStructure(1, { 0,1,5 });
    auto s600 = makeStructure(2, { 6,0,0 });

    SceneStructures structs;
    auto const& cStructs = structs;
    structs.insert(s010);
    structs.insert(s600);

    SECTION(".at()") {
        SECTION("// valid - const") {
            auto& res = cStructs.at(2);
            REQUIRE(&res == s600.get());
            CHECK(res.userData().isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto& res = structs.at(0);
            REQUIRE(&res == s010.get());
            CHECK_FALSE(res.userData().isCalledAsConst());
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(structs.at(1), std::out_of_range);
            CHECK_THROWS_AS(cStructs.at(3), std::out_of_range);
        }
    }

    SECTION(".atShared()") {
        SECTION("// valid - const") {
            auto res = cStructs.atShared(2);
            REQUIRE(res == s600);
            CHECK(res->userData().isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto res = structs.atShared(0);
            REQUIRE(res == s010);
            CHECK_FALSE(res->userData().isCalledAsConst());
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(structs.atShared(1), std::out_of_range);
            CHECK_THROWS_AS(cStructs.atShared(3), std::out_of_range);
        }
    }

    SECTION(".begin() // &.end()") {
        auto const expected = std::array{ s010, s600 };

        SECTION("// mutable") {
            CHECK_THAT(structs, matchers::c2::UnorderedRangeEquals(expected));
            auto const it = structs.begin();
            CHECK_FALSE((*it)->userData().isCalledAsConst());
        }

        SECTION("// const") {
            CHECK_THAT(cStructs, matchers::c2::UnorderedRangeEquals(expected));
            auto const it = cStructs.begin();
            CHECK((*it)->userData().isCalledAsConst());
        }
    }

    SECTION(".contains()") {
        SECTION("// true") {
            CHECK(cStructs.contains(0));
        }

        SECTION("// false") {
            CHECK_FALSE(cStructs.contains(1));
        }
    }

    SECTION(".erase()") {
        SECTION("// present") {
            auto const res = structs.erase(2);
            CHECK(res);
            CHECK(cStructs.size() == 1);
        }

        SECTION("// absent") {
            auto const res = structs.erase(1);
            CHECK_FALSE(res);
            CHECK(cStructs.size() == 2);
        }
    }

    SECTION(".findShared()") {
        SECTION("// valid - const") {
            auto res = cStructs.findShared(2);
            REQUIRE(res != nullptr);
            CHECK(res->userData().isCalledAsConst());
        }

        SECTION("// valid - mutable") {
            auto res = structs.findShared(0);
            REQUIRE(res != nullptr);
            CHECK_FALSE(res->userData().isCalledAsConst());
        }

        SECTION("// invalid") {
            auto const res = structs.findShared(1);
            CHECK(res == nullptr);
        }
    }

    SECTION(".insert()") {
        structs.insert(s015);
        CHECK(cStructs.size() == 3);
        CHECK(cStructs.contains(s015->index()));
    }

    SECTION(".size()") {
        CHECK(cStructs.size() == 2);
    }
}
