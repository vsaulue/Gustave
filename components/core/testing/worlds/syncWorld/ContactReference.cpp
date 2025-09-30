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

#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldUpdater.hpp>
#include <gustave/core/worlds/syncWorld/ContactReference.hpp>

#include <TestHelpers.hpp>

using ContactReference = gustave::core::worlds::syncWorld::ContactReference<libCfg>;
using WorldData = gustave::core::worlds::syncWorld::detail::WorldData<libCfg>;
using WorldUpdater = gustave::core::worlds::syncWorld::detail::WorldUpdater<libCfg>;

using BlockIndex = ContactReference::BlockReference::BlockIndex;
using BlockReference = ContactReference::BlockReference;
using ContactIndex = ContactReference::ContactIndex;
using Direction = ContactIndex::Direction;
using ForceStress = ContactReference::ForceStress;
using Solver = WorldData::Solver;
using StressRatio = ContactReference::StressRatio;
using StructureReference = ContactReference::StructureReference;
using Transaction = WorldUpdater::Transaction;

static constexpr auto blockSize = vector3(3.f, 2.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static WorldData makeWorld() {
    auto solver = Solver{ Solver::Config{ g, solverPrecision } };
    return WorldData{ blockSize, std::move(solver) };
}

TEST_CASE("core::worlds::syncWorld::ContactReference") {
    WorldData world = makeWorld();

    {
        Transaction t;
        t.addBlock({ {2,2,2}, concrete_20m, blockMass, false });
        t.addBlock({ {2,1,2}, concrete_20m, blockMass, true });
        t.addBlock({ {4,1,4}, concrete_20m, blockMass, false });
        t.addBlock({ {4,2,4}, concrete_20m, blockMass, false });
        t.addBlock({ {6,6,6}, concrete_20m, blockMass, true });
        t.addBlock({ {7,5,6}, concrete_20m, blockMass, false });
        t.addBlock({ {7,6,6}, concrete_20m, blockMass, false });
        t.addBlock({ {7,7,6}, concrete_20m, blockMass, false });
        WorldUpdater{ world }.runTransaction(t);
    }

    auto makeContactRef = [&](BlockIndex const& sourceBlockId, Direction direction) {
        return ContactReference{ world, ContactIndex{ sourceBlockId, direction } };
    };

    auto const contactId = ContactIndex{ {2,2,2}, Direction::minusY() };
    auto const contact = ContactReference{ world, contactId };
    auto const invalidContact = makeContactRef({ 2,2,2 }, Direction::plusZ());
    auto const unsolvedContact = makeContactRef({ 4,1,4 }, Direction::plusY());

    auto const c5 = makeContactRef({ 7,5,6 }, Direction::plusY());
    auto const c6 = makeContactRef({ 7,6,6 }, Direction::minusX());
    auto const c7 = makeContactRef({ 7,7,6 }, Direction::minusY());

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

    SECTION(".forceStress()") {
        auto checkForceStress = [&](ForceStress const& value, ForceStress const& expected) {
            CHECK_THAT(value.compression(), matchers::WithinRel(expected.compression(), solverPrecision));
            CHECK_THAT(value.shear(), matchers::WithinRel(expected.shear(), solverPrecision));
            CHECK_THAT(value.tensile(), matchers::WithinRel(expected.tensile(), solverPrecision));
        };

        SECTION("// compression") {
            auto const expected = ForceStress{
                blockMass * g.norm(),
                0.f * u.force,
                0.f * u.force,
            };
            checkForceStress(c7.forceStress(), expected);
            checkForceStress(c7.opposite().forceStress(), expected);
        }

        SECTION("// shear") {
            auto const expected = ForceStress{
                0.f * u.force,
                3.f * blockMass * g.norm(),
                0.f * u.force,
            };
            checkForceStress(c6.forceStress(), expected);
            checkForceStress(c6.opposite().forceStress(), expected);
        }

        SECTION("// tensile") {
            auto const expected = ForceStress{
                0.f * u.force,
                0.f * u.force,
                blockMass * g.norm(),
            };
            checkForceStress(c5.forceStress(), expected);
            checkForceStress(c5.opposite().forceStress(), expected);
        }
    }

    SECTION(".forceVector()") {
        SECTION("// valid") {
            CHECK_THAT(contact.forceVector(), matchers::WithinRel(-blockMass * g, solverPrecision));
        }

        SECTION("// invalid (contact doesn't exist)") {
            CHECK_THROWS_AS(invalidContact.forceVector(), std::out_of_range);
        }

        SECTION("// unsolved") {
            CHECK_THROWS_AS(unsolvedContact.forceVector(), std::logic_error);
        }
    }

    SECTION(".index()") {
        CHECK(contact.index() == contactId);
    }

    SECTION(".isSolved()") {
        SECTION("// solved") {
            CHECK(contact.isSolved());
        }

        SECTION("// not solved") {
            CHECK_FALSE(unsolvedContact.isSolved());
        }

        SECTION("// invalid") {
            CHECK_FALSE(invalidContact.isSolved());
        }
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

    SECTION(".maxPressureStress()") {
        SECTION("// valid") {
            CHECK(contact.maxPressureStress() == concrete_20m);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(invalidContact.maxPressureStress(), std::out_of_range);
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

    SECTION(".otherBlock()") {
        CHECK(contact.otherBlock() == makeBlockRef({ 2,1,2 }));
    }

    SECTION(".pressureStress()") {
        auto checkPressureStress = [&](PressureStress const& value, PressureStress const& expected) {
            CHECK_THAT(value.compression(), matchers::WithinRel(expected.compression(), solverPrecision));
            CHECK_THAT(value.shear(), matchers::WithinRel(expected.shear(), solverPrecision));
            CHECK_THAT(value.tensile(), matchers::WithinRel(expected.tensile(), solverPrecision));
        };

        SECTION("// compression") {
            auto const expected = PressureStress{
                blockMass * g.norm() / c7.area(),
                0.f * u.pressure,
                0.f * u.pressure,
            };
            checkPressureStress(c7.pressureStress(), expected);
            checkPressureStress(c7.opposite().pressureStress(), expected);
        }

        SECTION("// shear") {
            auto const expected = PressureStress{
                0.f * u.pressure,
                3.f * blockMass * g.norm() / c6.area(),
                0.f * u.pressure,
            };
            checkPressureStress(c6.pressureStress(), expected);
            checkPressureStress(c6.opposite().pressureStress(), expected);
        }

        SECTION("// tensile") {
            auto const expected = PressureStress{
                0.f * u.pressure,
                0.f * u.pressure,
                blockMass * g.norm() / c5.area(),
            };
            checkPressureStress(c5.pressureStress(), expected);
            checkPressureStress(c5.opposite().pressureStress(), expected);
        }
    }

    SECTION(".stressRatio()") {
        auto checkStressRatio = [&](StressRatio const& value, StressRatio const& expected) {
            CHECK_THAT(value.compression(), matchers::WithinRel(expected.compression(), solverPrecision));
            CHECK_THAT(value.shear(), matchers::WithinRel(expected.shear(), solverPrecision));
            CHECK_THAT(value.tensile(), matchers::WithinRel(expected.tensile(), solverPrecision));
        };

        SECTION("// compression") {
            auto const expected = StressRatio{
                blockMass * g.norm() / (concrete_20m.compression() * c7.area()),
                0.f,
                0.f,
            };
            checkStressRatio(c7.stressRatio(), expected);
            checkStressRatio(c7.opposite().stressRatio(), expected);
        }

        SECTION("// shear") {
            auto const expected = StressRatio{
                0.f,
                3.f * blockMass * g.norm() / (concrete_20m.shear() * c6.area()),
                0.f,
            };
            checkStressRatio(c6.stressRatio(), expected);
            checkStressRatio(c6.opposite().stressRatio(), expected);
        }

        SECTION("// tensile") {
            auto const expected = StressRatio{
                0.f,
                0.f,
                blockMass * g.norm() / (concrete_20m.tensile() * c5.area()),
            };
            checkStressRatio(c5.stressRatio(), expected);
            checkStressRatio(c5.opposite().stressRatio(), expected);
        }
    }

    SECTION(".structure()") {
        SECTION("// valid") {
            auto sceneStructure = world.scene.blocks().at({ 2,2,2 }).structures()[0];
            CHECK(contact.structure() == StructureReference{ world.structures.at(sceneStructure.index()) });
        }
    }
}
