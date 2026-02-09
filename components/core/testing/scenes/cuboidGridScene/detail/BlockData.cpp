/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/utils/IndexGenerator.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

using BlockData = gustave::core::scenes::cuboidGridScene::detail::BlockData<libCfg, SceneUserData>;

using BlockConstructionInfo = BlockData::BlockConstructionInfo;
using BlockIndex = BlockData::BlockIndex;
using LinkIndex = BlockData::LinkIndex;
using SceneData = BlockData::SceneData;
using StructureIdGenerator = gustave::utils::IndexGenerator<BlockData::StructureIndex>;

TEST_CASE("core::scenes::cuboidGridScene::detail::BlockData") {
    auto scene = SceneData{ vector3(1.f, 2.f, 3.f, u.length) };
    auto b111 = BlockData{ BlockConstructionInfo{ {1,1,1}, concrete_20m, 5.f * u.mass, false }, scene };
    auto const& cb111 = b111;

    SECTION("// constructor && const getters") {
        CHECK(cb111.index() == BlockIndex{ 1,1,1 });
        CHECK(cb111.mass() == 5.f * u.mass);
        CHECK(cb111.isFoundation() == false);
        CHECK(cb111.isValid() == true);
        CHECK(&cb111.sceneData() == &scene);
    }

    SECTION(".invalidate()") {
        b111.invalidate();
        CHECK_FALSE(cb111.isValid());
    }

    SECTION(".linkIndices()") {
        constexpr auto maxLinkId = std::numeric_limits<LinkIndex>::max();
        auto const& cLinkIds = cb111.linkIndices();

        SECTION("// mutable") {
            b111.linkIndices().plusX = 64;
            CHECK(cLinkIds.plusX == 64);
            CHECK(cLinkIds.plusY == maxLinkId);
            CHECK(cLinkIds.plusZ == maxLinkId);
        }

        SECTION("// const") {
            CHECK(cLinkIds.plusX == maxLinkId);
            CHECK(cLinkIds.plusY == maxLinkId);
            CHECK(cLinkIds.plusZ == maxLinkId);
        }
    }

    SECTION(".setSceneData()") {
        auto scene2 = SceneData{ vector3(2.f, 3.f, 4.f, u.length) };
        b111.setSceneData(scene2);
        CHECK(&cb111.sceneData() == &scene2);
    }

    SECTION(".structureId()") {
        SECTION("// mutable") {
            b111.structureId() = 64;
            CHECK(cb111.structureId() == 64);
        }

        SECTION("// const") {
            CHECK(cb111.structureId() == StructureIdGenerator::invalidIndex());
        }
    }

    SECTION(".userData()") {
        SECTION("// mutable") {
            CHECK_FALSE(b111.userData().isCalledAsConst());
            b111.userData().tag = 0.25f;
            CHECK(cb111.userData().tag == 0.25f);
        }

        SECTION("// const") {
            CHECK(cb111.userData().isCalledAsConst());
        }
    }
}
