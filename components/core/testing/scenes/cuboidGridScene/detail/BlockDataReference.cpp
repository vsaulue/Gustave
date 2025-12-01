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

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/BlockConstructionInfo.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockDataReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

using BlockConstructionInfo = gustave::core::scenes::cuboidGridScene::BlockConstructionInfo<libCfg>;
using BlockData = gustave::core::scenes::cuboidGridScene::detail::BlockData<libCfg, SceneUserData>;
using BlockIndex = gustave::core::scenes::cuboidGridScene::BlockIndex;
using BlockMappedData = gustave::core::scenes::cuboidGridScene::detail::BlockMappedData<libCfg, SceneUserData>;
using SceneData = gustave::core::scenes::cuboidGridScene::detail::SceneData<libCfg, SceneUserData>;

template<bool mut>
using BlockDataReference = gustave::core::scenes::cuboidGridScene::detail::BlockDataReference<libCfg, SceneUserData, mut>;

TEST_CASE("core::scenes::cuboidGridScene::detail::BlockDataReference") {
    SceneData sceneData{ vector3(1.f, 1.f, 1.f, u.length) };

    auto mb111 = sceneData.blocks.insert(BlockConstructionInfo{ {1,1,1}, concrete_20m, 5.f * u.mass, false });
    auto const& cmb111 = mb111;
    auto ib111 = BlockDataReference<false>{ mb111 };

    auto mb333 = sceneData.blocks.insert(BlockConstructionInfo{ {3,3,3}, concrete_20m, 10.f * u.mass, true });
    auto ib333 = BlockDataReference<false>{ mb333 };

    auto mInvalid = BlockDataReference<true>{ nullptr };
    auto iInvalid = BlockDataReference<false>{ nullptr };

    SECTION("// constructor && const getters") {
        auto runTest = [&](auto&& block) {
            CHECK(block.index() == BlockIndex{ 1,1,1 });
            CHECK(block.mass() == 5.f * u.mass);
            CHECK(block.isFoundation() == false);
        };

        SECTION("// mutable") {
            runTest(mb111);
        }

        SECTION("// immutable") {
            runTest(ib111);
        }
    }

    SECTION(".structureId()") {
        SECTION("// immutable") {
            CHECK(ib333.structureId() == sceneData.structureIdGenerator.invalidIndex());
        }

        SECTION("// mutable") {
            mb333.structureId() = 64;
            CHECK(ib333.structureId() == 64);
        }
    }

    SECTION(".userData()") {
        SECTION("// mutable") {
            CHECK_FALSE(mb111.userData().isCalledAsConst());
            mb111.userData().tag = 0.25f;
            CHECK(ib111.userData().tag == 0.25f);
        }

        SECTION("// const") {
            CHECK(cmb111.userData().isCalledAsConst());
        }

        SECTION("// immutable") {
            CHECK(ib111.userData().isCalledAsConst());
        }
    }

    SECTION(".operator bool() const") {
        SECTION("operator bool() const") {
            SECTION("// true") {
                CHECK(mb111);
                CHECK(ib111);
            }

            SECTION("// false") {
                CHECK_FALSE(mInvalid);
                CHECK_FALSE(iInvalid);
            }
        }
    }

    SECTION("::operator==()") {
        SECTION("// mutable == mutable") {
            CHECK(mb111 == BlockDataReference<true>{ mb111 });
            CHECK(mb333 != mb111);
            CHECK(mb111 != mInvalid);
        }

        SECTION("// mutable == immutable") {
            CHECK(mb111 == ib111);
            CHECK(ib333 == mb333);
            CHECK(mb111 != ib333);
            CHECK(ib111 != mb333);
            CHECK(mb111 != iInvalid);
            CHECK(mInvalid != ib111);
        }

        SECTION("// immutable == immutable") {
            CHECK(ib111 == BlockDataReference<false>{ mb111 });
            CHECK(ib111 != ib333);
            CHECK(ib111 != iInvalid);
        }
    }
}
