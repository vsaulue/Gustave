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

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockData.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>

#include <TestHelpers.hpp>

using BlockConstructionInfo = gustave::scenes::cuboidGrid::BlockConstructionInfo<libCfg>;
using BlockData = gustave::scenes::cuboidGrid::detail::BlockData<libCfg>;
using BlockIndex = gustave::scenes::cuboidGrid::BlockIndex;
using BlockMappedData = gustave::scenes::cuboidGrid::detail::BlockMappedData<libCfg>;
using SceneData = gustave::scenes::cuboidGrid::detail::SceneData<libCfg>;
using StructureData = gustave::scenes::cuboidGrid::detail::StructureData<libCfg>;

template<bool isMutable>
using BlockDataReference = gustave::scenes::cuboidGrid::detail::BlockDataReference<libCfg, isMutable>;

TEST_CASE("scenes::cuboidGrid::detail::BlockDataReference") {
    SceneData sceneData{ vector3(1.f, 1.f, 1.f, u.length) };

    BlockDataReference<true> b111 = sceneData.blocks.insert(BlockConstructionInfo{ {1,1,1}, concrete_20m, 5.f * u.mass, false });
    BlockDataReference<true> b333 = sceneData.blocks.insert(BlockConstructionInfo{ {3,3,3}, concrete_20m, 10.f * u.mass, true });

    SECTION("<*,true>") {
        SECTION("// constructor & getters") {
            CHECK(b111.index() == BlockIndex{ 1,1,1 });
            CHECK(b111.mass() == 5.f * u.mass);
            CHECK(b111.isFoundation() == false);
            CHECK(b111.structure() == nullptr);
        }

        SECTION("operator bool() const") {
            SECTION("// true") {
                CHECK(b111);
            }

            SECTION("// false") {
                BlockDataReference<true> const ref{ nullptr };
                CHECK_FALSE(ref);
            }
        }

        SECTION(".structure() // mutable") {
            StructureData structure{ sceneData, b111 };
            CHECK(b333.structure() == nullptr);
            b333.structure() = &structure;
            CHECK(b333.structure() == &structure);
        }
    }

    SECTION("<*,false>") {
        SECTION("::BlockDataReference(BlockData const*) // + getters") {
            BlockDataReference<false> cRef{ b111.data() };
            CHECK(cRef.index() == BlockIndex{ 1,1,1 });
            CHECK(cRef.mass() == 5.f * u.mass);
            CHECK(cRef.isFoundation() == false);
            CHECK(cRef.structure() == nullptr);
        }

        SECTION("::BlockDataReference(BlockDataReference<*,true> const&)") {
            BlockDataReference<false> cRef{ b111 };
            CHECK(b111.data() == cRef.data());
        }

        SECTION("::operator==(BlockDataReference<*;true> const&)") {
            BlockDataReference<true> nullRef{ nullptr };
            BlockDataReference<false> cRef{ b111.data() };
            CHECK(cRef == b111);
            CHECK(cRef != nullRef);
        }
    }
}
