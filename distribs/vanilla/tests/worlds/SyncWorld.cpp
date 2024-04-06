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

#include <gustave/worlds/SyncWorld.hpp>

#include <TestHelpers.hpp>

using SyncWorld = gustave::worlds::SyncWorld<libCfg>;

using BlockIndex = SyncWorld::BlockIndex;
using ContactIndex = SyncWorld::ContactIndex;
using Direction = SyncWorld::ContactIndex::Direction;
using Solver = SyncWorld::Solver;

static constexpr auto blockSize = vector3(1.f, 1.f, 1.f, u.length);
static constexpr Real<u.density> concreteDensity = 2'400.f * u.density;
static constexpr Real<u.mass> blockMass = blockSize.x() * blockSize.y() * blockSize.z() * concreteDensity;
static constexpr float solverPrecision = 0.001f;

[[nodiscard]]
static SyncWorld makeWorld() {
    auto const g = vector3(0.f, -10.f, 0.f, u.acceleration);
    auto solver = Solver{ std::make_shared<Solver::Config>(g, 1000, solverPrecision) };
    return SyncWorld{ blockSize, std::move(solver) };
}

TEST_CASE("worlds::SyncWorld") {
    SyncWorld world = makeWorld();
    {
        SyncWorld::Transaction transaction;
        for (int i = 0; i < 10; ++i) {
            transaction.addBlock({ {0,i,0}, concrete_20m, blockMass, i == 0 });
        }
        world.modify(transaction);
    }

    SECTION(".blocks()") {
        auto const block = world.blocks().at({ 0,2,0 });
        CHECK(block.mass() == blockMass);
    }

    SECTION(".contacts()") {
        auto const contact = world.contacts().at(ContactIndex{ {0,0,0}, Direction::plusY() });
        CHECK_THAT(contact.forceVector(), matchers::WithinRel(9.f * blockMass * g, solverPrecision));
    }

    SECTION(".links()") {
        std::vector<ContactIndex> expected;
        for (int i = 0; i < 9; ++i) {
            expected.push_back(ContactIndex{ {0,i,0}, Direction::plusY() });
        }
        std::vector<ContactIndex> result;
        for (auto const& contactRef : world.links()) {
            result.push_back(contactRef.index());
        }
        CHECK_THAT(result, matchers::c2::UnorderedRangeEquals(expected));
    }

    SECTION(".structures()") {
        auto const structureIt = world.structures().begin();
        REQUIRE(structureIt != world.structures().end());
        CHECK(structureIt->blocks().size() == 10);
    }
}
