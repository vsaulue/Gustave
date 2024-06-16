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

#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>

#include <TestHelpers.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg>;

using BlockConstructionInfo = SceneData::Blocks::BlockConstructionInfo;
using SceneBlocks = SceneData::Blocks;
using StructureData = SceneData::StructureData;

TEST_CASE("core::scenes::cuboidGridScene::detail::SceneData") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    auto scene1 = SceneData{ blockSize };

    auto block1 = scene1.blocks.insert({ {1,1,1}, concrete_20m, 1000.f * u.mass, false });
    auto struct1 = std::make_shared<StructureData>(scene1, block1);
    scene1.structures.insert(struct1);

    SECTION("// move semantics") {
        auto const expectedStructs = std::vector<std::shared_ptr<StructureData>>{
            struct1,
        };

        auto checkMovedScene = [&](SceneData const& movedScene) {
            CHECK_THAT(movedScene.structures, matchers::c2::UnorderedRangeEquals(expectedStructs));
            CHECK(&struct1->sceneData() == &movedScene);
            CHECK(movedScene.blocks.blockSize() == blockSize);
        };

        SECTION("// assign") {
            auto scene2 = SceneData{ blockSize };
            scene2 = std::move(scene1);

            checkMovedScene(scene2);
        }

        SECTION("// constructor") {
            auto scene2 = SceneData{ std::move(scene1) };

            checkMovedScene(scene2);
        }
    }
}
