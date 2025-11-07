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

#include <array>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/CuboidGridScene.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <TestHelpers.hpp>

namespace {
    struct UserData {
        using Structure = gustave::testing::ConstDetector<int>;
    };
}

using Scene = gustave::core::scenes::CuboidGridScene<libCfg, UserData>;

using BlockIndex = Scene::BlockIndex;
using Direction = Scene::Direction;
using Transaction = Scene::Transaction;

static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;

TEST_CASE("core::scenes::CuboidGridScene") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;

    Scene scene{blockSize};
    auto const& cScene = scene;

    Transaction t;
    t.addBlock({ {0,0,0}, concrete_20m, 1.f * blockMass, false });
    t.addBlock({ {1,0,0}, concrete_20m, 2.f * blockMass, true });
    t.addBlock({ {2,0,0}, concrete_20m, 3.f * blockMass, false });
    scene.modify(t);

    SECTION(".blocks()") {
        auto runTest = [&](auto&& blocks, bool expectedConst) {
            REQUIRE(blocks.size() == 3);
            CHECK(expectedConst == blocks.at({ 0,0,0 }).structures()[0].userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(scene.blocks(), false);
        }

        SECTION("// const") {
            runTest(cScene.blocks(), true);
        }
    }

    SECTION(".blockSize() const") {
        CHECK(scene.blockSize() == blockSize);
    }

    SECTION(".contacts()") {
        auto runTest = [&](auto&& contacts, bool expectedConst) {
            auto contact = contacts.at({ {1,0,0}, Direction::plusX() });
            CHECK(contact.maxPressureStress() == concrete_20m);
            CHECK(contact.otherBlock().mass() == 3.f * blockMass);
            CHECK(expectedConst == contact.structure().userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(scene.contacts(), false);
        }

        SECTION("// const") {
            runTest(cScene.contacts(), true);
        }
    }

    SECTION(".links()") {
        auto links = scene.links();

        auto const expected = std::array{
            scene.contacts().at({ {0,0,0}, Direction::plusX() }),
            scene.contacts().at({ {1,0,0}, Direction::plusX() }),
        };
        CHECK_THAT(links, matchers::c2::UnorderedRangeEquals(expected));
    }

    SECTION(".structures()") {
        auto runTest = [&](auto&& structs, bool expectedConst) {
            REQUIRE(structs.size() == 2);
            CHECK(expectedConst == structs.begin()->userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(scene.structures(), false);
        }

        SECTION("// const") {
            runTest(cScene.structures(), true);
        }
    }
}
