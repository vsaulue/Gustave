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

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockData.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>

#include <TestHelpers.hpp>

using BlockConstructionInfo = Gustave::Scenes::CuboidGrid::BlockConstructionInfo<cfg>;
using BlockData = Gustave::Scenes::CuboidGrid::detail::BlockData<cfg>;
using SceneData = Gustave::Scenes::CuboidGrid::detail::SceneData<cfg>;
using StructureData = Gustave::Scenes::CuboidGrid::detail::StructureData<cfg>;

template<bool isMutable>
using BlockDataReference = Gustave::Scenes::CuboidGrid::detail::BlockDataReference<cfg, isMutable>;

TEST_CASE("Scenes::CuboidGrid::detail::BlockDataReference") {
    BlockConstructionInfo const info{ {4,5,6}, concrete_20m, 5.f * u.mass, true };
    BlockData data{ info.index(), { info } };
    SceneData const sceneData{ vector3(1.f, 1.f, 1.f, u.length) };

    SECTION("<*,true>") {
        SECTION("// constructor & getters") {
            BlockDataReference<true> ref{ &data };
            CHECK(ref.index() == info.index());
            CHECK(ref.mass() == 5.f * u.mass);
            CHECK(ref.isFoundation() == true);
            CHECK(ref.structure() == nullptr);
        }

        SECTION("operator bool() const") {
            SECTION("// true") {
                BlockDataReference<true> const ref{ &data };
                CHECK(ref);
            }

            SECTION("// false") {
                BlockDataReference<true> const ref{ nullptr };
                CHECK_FALSE(ref);
            }
        }

        SECTION("::structure() // mutable") {
            BlockDataReference<true> ref{ &data };
            StructureData structure{ sceneData };
            ref.structure() = &structure;
            CHECK(data.second.structure() == &structure);
        }
    }

    SECTION("<*,false>") {
        BlockData const* const cData = &data;

        SECTION("::BlockDataReference(BlockData const*) // + getters") {
            BlockDataReference<false> cRef{ cData };
            CHECK(cRef.index() == info.index());
            CHECK(cRef.mass() == 5.f * u.mass);
            CHECK(cRef.isFoundation() == true);
            CHECK(cRef.structure() == nullptr);
        }

        SECTION("::BlockDataReference(BlockDataReference<*,true> const&)") {
            BlockDataReference<true> ref{ &data };
            BlockDataReference<false> cRef{ ref };
            CHECK(ref.data() == cRef.data());
        }

        SECTION("::operator==(BlockDataReference<*;true> const&)") {
            BlockDataReference<true> validRef{ &data };
            BlockDataReference<true> nullRef{ nullptr };
            BlockDataReference<false> cRef{ cData };

            CHECK(cRef == validRef);
            CHECK(cRef != nullRef);
        }
    }
}
