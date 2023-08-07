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
#include <gustave/scenes/cuboidGrid/detail/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>
#include <gustave/scenes/cuboidGrid/SceneStructure.hpp>

#include <TestConfig.hpp>

using BlockConstructionInfo = Gustave::Scenes::CuboidGrid::BlockConstructionInfo<G::libConfig>;
using BlockData = Gustave::Scenes::CuboidGrid::detail::BlockData<G::libConfig>;
using SceneBlocks = Gustave::Scenes::CuboidGrid::detail::SceneBlocks<G::libConfig>;
using SceneStructure = Gustave::Scenes::CuboidGrid::SceneStructure<G::libConfig>;

template<bool isMutable>
using BlockReference = Gustave::Scenes::CuboidGrid::detail::BlockReference<G::libConfig, isMutable>;

TEST_CASE("Scenes::CuboidGrid::detail::BlockReference") {
    BlockConstructionInfo const info{ {4,5,6}, concrete_20m, 5.f * u.mass, true };
    BlockData data{ info.position(), { info } };
    SceneBlocks const sceneBlocks;

    SECTION("<*,true>") {
        SECTION("// constructor & getters") {
            BlockReference<true> ref{ &data };
            CHECK(ref.position() == info.position());
            CHECK(ref.mass() == 5.f * u.mass);
            CHECK(ref.isFoundation() == true);
            CHECK(ref.structure() == nullptr);
        }

        SECTION("operator bool() const") {
            SECTION("// true") {
                BlockReference<true> const ref{ &data };
                CHECK(ref);
            }

            SECTION("// false") {
                BlockReference<true> const ref{ nullptr };
                CHECK_FALSE(ref);
            }
        }

        SECTION("::structure() // mutable") {
            BlockReference<true> ref{ &data };
            SceneStructure structure{ sceneBlocks };
            ref.structure() = &structure;
            CHECK(data.second.structure() == &structure);
        }
    }

    SECTION("<*,false>") {
        BlockData const* const cData = &data;

        SECTION("::BlockReference(BlockData const*) // + getters") {
            BlockReference<false> cRef{ cData };
            CHECK(cRef.position() == info.position());
            CHECK(cRef.mass() == 5.f * u.mass);
            CHECK(cRef.isFoundation() == true);
            CHECK(cRef.structure() == nullptr);
        }

        SECTION("::BlockReference(BlockReference<*,true> const&)") {
            BlockReference<true> ref{ &data };
            BlockReference<false> cRef{ ref };
            CHECK(ref.data() == cRef.data());
        }

        SECTION("::operator==(BlockReference<*;true> const&)") {
            BlockReference<true> validRef{ &data };
            BlockReference<true> nullRef{ nullptr };
            BlockReference<false> cRef{ cData };

            CHECK(cRef == validRef);
            CHECK(cRef != nullRef);
        }
    }
}
