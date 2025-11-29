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

#include <limits>

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockMappedData.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

using BlockConstructionInfo = gustave::core::scenes::cuboidGridScene::BlockConstructionInfo<libCfg>;
using BlockMappedData = gustave::core::scenes::cuboidGridScene::detail::BlockMappedData<libCfg, SceneUserData>;

using IndexGenerator = gustave::utils::IndexGenerator<std::size_t>;

TEST_CASE("core::scenes::cuboidGridScene::detail::BlockMappedData") {
    auto const info = BlockConstructionInfo{ {4,5,6}, concrete_20m, 5.f * u.mass, true };
    auto data = BlockMappedData{ info };
    auto const& cData = data;

    constexpr LinkIndex maxLinkId = std::numeric_limits<LinkIndex>::max();

    SECTION("// constructor & getters") {
        CHECK(cData.maxPressureStress() == concrete_20m);
        CHECK(cData.linkIndices().plusX == maxLinkId);
        CHECK(cData.linkIndices().plusY == maxLinkId);
        CHECK(cData.linkIndices().plusZ == maxLinkId);
        CHECK(cData.mass() == 5.f * u.mass);
        CHECK(cData.isFoundation() == true);
        CHECK(cData.structureId() == IndexGenerator::invalidIndex());
    }

    SECTION(".userData()") {
        SECTION("// mutable") {
            CHECK_FALSE(data.userData().isCalledAsConst());
        }

        SECTION("// const") {
            CHECK(cData.userData().isCalledAsConst());
        }
    }
}
