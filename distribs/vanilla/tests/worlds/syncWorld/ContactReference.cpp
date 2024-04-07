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

#include <gustave/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/worlds/syncWorld/detail/WorldUpdater.hpp>
#include <gustave/worlds/syncWorld/ContactReference.hpp>

#include <TestHelpers.hpp>

using ContactReference = gustave::worlds::syncWorld::ContactReference<libCfg>;
using WorldData = gustave::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::worlds::syncWorld::detail::WorldUpdater<libCfg>;

using BlockIndex = ContactReference::BlockReference::BlockIndex;
using BlockReference = ContactReference::BlockReference;
using ContactIndex = ContactReference::ContactIndex;
using Direction = ContactIndex::Direction;
using Solver = WorldData::Solver;
using StructureReference = ContactReference::StructureReference;
using Transaction = WorldUpdater::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ std::make_shared<Solver::Config>(g, 1000, solverPrecision) };
    return WorldData{ blockSize, std::move(solver) };
}

TEST_CASE("worlds::syncWorld::ContactReference") {
    WorldData world = makeWorld();

    Transaction t;
    t.addBlock({ {2,2,2}, concrete_20m, blockMass, false });
    t.addBlock({ {2,1,2}, concrete_20m, blockMass, true });
    WorldUpdater{ world }.runTransaction(t);

    auto const contactId = ContactIndex{ {2,2,2}, Direction::minusY() };
    auto const contact = ContactReference{ world, contactId };
    auto const invalidContact = ContactReference{ world, ContactIndex{ {2,2,2}, Direction::plusZ() } };

    auto makeBlockRef = [&](BlockIndex const& index) {
        return BlockReference{ world, index };
    };

    SECTION(".area()") {
        SECTION("// valid") {
            CHECK(contact.area() == 3.f * u.area);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.area(), std::out_of_range);
        }
    }

    SECTION(".forceVector()") {
        SECTION("// valid") {
            CHECK_THAT(contact.forceVector(), matchers::WithinRel(-blockMass * g, solverPrecision));
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.forceVector(), std::out_of_range);
        }
    }

    SECTION(".index()") {
        CHECK(contact.index() == contactId);
    }

    SECTION(".isValid()") {
        SECTION("// true") {
            CHECK(contact.isValid());
        }

        SECTION("// false") {
            CHECK_FALSE(invalidContact.isValid());
        }
    }

    SECTION(".localBlock()") {
        CHECK(contact.localBlock() == makeBlockRef({ 2,2,2 }));
    }

    SECTION(".maxStress()") {
        SECTION("// valid") {
            CHECK(contact.maxStress() == concrete_20m);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.maxStress(), std::out_of_range);
        }
    }

    SECTION(".normal()") {
        SECTION("// valid") {
            CHECK(contact.normal() == -Normals::y);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.normal(), std::out_of_range);
        }
    }

    SECTION(".opposite()") {
        SECTION("// valid") {
            auto const expectedId = ContactIndex{ {2,1,2}, Direction::plusY() };
            CHECK(contact.opposite() == ContactReference{ world, expectedId });
        }

        SECTION("// invalid") {
            static constexpr auto coordMax = std::numeric_limits<BlockIndex::Coord>::max();
            auto const limitContact = ContactReference{ world, ContactIndex{ {0,coordMax,0}, Direction::plusY() } };
            CHECK_THROWS_AS(limitContact.opposite(), std::out_of_range);
        }
    }

    SECTION(".structure()") {
        SECTION("// valid") {
            auto sceneStructure = world.scene.blocks().at({ 2,2,2 }).structures()[0];
            CHECK(contact.structure() == StructureReference{ world.structures.at(sceneStructure) });
        }
    }
}
