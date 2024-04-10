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

#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include <gustave/scenes/CuboidGridScene.hpp>

#include <TestHelpers.hpp>

using Scene = gustave::scenes::CuboidGridScene<libCfg>;

using BlockIndex = Scene::BlockIndex;
using Direction = Scene::Direction;
using Transaction = Scene::Transaction;

static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;

TEST_CASE("scenes::CuboidGridScene") {
    auto const blockSize = vector3(1.f, 2.f, 3.f, u.length);
    Real<u.mass> const blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;

    Scene scene{blockSize};

    SECTION(".blocks() const") {
        Scene::Blocks blocks = scene.blocks();

        SECTION("// empty") {
            CHECK(blocks.size() == 0);
            CHECK(blocks.begin() == blocks.end());
        }

        SECTION("// not empty") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {2,0,0}, concrete_20m, blockMass, false });
            scene.modify(t);

            CHECK(blocks.size() == 2);
            CHECK(blocks.at({1,0,0}).isFoundation());
            CHECK_FALSE(blocks.at({ 2,0,0 }).isFoundation());

            {
                std::vector<BlockIndex> indices;
                for (auto const& block : blocks) {
                    indices.push_back(block.index());
                }
                std::vector<BlockIndex> expected{ {1,0,0}, {2,0,0} };
                CHECK_THAT(indices, matchers::c2::UnorderedEquals(expected));
            }
        }
    }

    SECTION(".blockSize() const") {
        CHECK(scene.blockSize() == blockSize);
    }

    SECTION(".contacts()") {
        Scene::Contacts contacts = scene.contacts();

        Transaction t;
        t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
        t.addBlock({ {2,0,0}, concrete_20m, 5.f * blockMass, false });
        scene.modify(t);

        Scene::ContactReference contact = contacts.at(Scene::ContactIndex{ {1,0,0}, Direction::plusX() });
        CHECK(contact.maxPressureStress() == concrete_20m);
        CHECK(contact.otherBlock().mass() == 5.f * blockMass);
    }

    SECTION(".links()") {
        Scene::Links links = scene.links();

        Transaction t;
        t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
        t.addBlock({ {2,0,0}, concrete_20m, blockMass, false });
        t.addBlock({ {3,0,0}, concrete_20m, blockMass, false });
        scene.modify(t);

        std::vector<Scene::ContactReference> const expected = {
            scene.contacts().at(Scene::ContactIndex{ {1,0,0}, Direction::plusX() }),
            scene.contacts().at(Scene::ContactIndex{ {2,0,0}, Direction::plusX() }),
        };
        CHECK_THAT(links, matchers::c2::UnorderedRangeEquals(expected));
    }

    SECTION(".structures() const") {
        Scene::Structures structures = scene.structures();

        SECTION("// empty") {
            CHECK(structures.size() == 0);
            CHECK(structures.begin() == structures.end());
        }

        SECTION("// not empty") {
            Transaction t;
            t.addBlock({ {1,0,0}, concrete_20m, blockMass, false });
            t.addBlock({ {2,0,0}, concrete_20m, blockMass, true });
            t.addBlock({ {3,0,0}, concrete_20m, blockMass, false });
            scene.modify(t);

            CHECK(structures.size() == 2);
            int count = 0;
            for (auto const& structure : structures) {
                CHECK(structure.blocks().contains({ 2,0,0 }));
                ++count;
            }
            CHECK(count == 2);
        }
    }
}
