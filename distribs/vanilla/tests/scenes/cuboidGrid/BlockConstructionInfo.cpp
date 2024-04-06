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

#include <exception>

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>

#include <TestHelpers.hpp>

namespace cuboid = ::gustave::scenes::cuboidGrid;

using BlockConstructionInfo = cuboid::BlockConstructionInfo<libCfg>;
using BlockIndex = cuboid::BlockIndex;

TEST_CASE("scenes::cuboidGrid::BlockConstructionInfo") {
    SECTION("// valid constructor & getters") {
        const BlockConstructionInfo obj{ {4,5,6}, concrete_20m, 5.f * u.mass, true };
        CHECK(obj.index() == BlockIndex{4, 5, 6});
        CHECK(obj.maxStress().maxTensileStress() == concrete_20m.maxTensileStress());
        CHECK(obj.mass() == 5.f * u.mass);
        CHECK(obj.isFoundation() == true);
    }

    SECTION("// constructor: invalid mass") {
        auto constructor = []() {
            return BlockConstructionInfo{ {4,5,6}, concrete_20m, 0.f * u.mass, true };
        };
        CHECK_THROWS_AS(constructor(), std::invalid_argument);
    }

    SECTION(".setMass(Real<u.mass>)") {
        BlockConstructionInfo obj{ {4,5,6}, concrete_20m, 5.f * u.mass, true };

        SECTION("// valid") {
            obj.setMass(2.f * u.mass);
            CHECK(obj.mass() == 2.f * u.mass);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(obj.setMass(-1.f * u.mass), std::invalid_argument);
        }
    }
}
