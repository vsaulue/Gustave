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

#include <memory>

#include <gustave/worlds/sync/Contacts.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>
#include <gustave/worlds/sync/detail/WorldUpdater.hpp>

#include <TestHelpers.hpp>

using Contacts = gustave::worlds::sync::Contacts<libCfg>;
using WorldData = gustave::worlds::sync::detail::WorldData<libCfg>;
using WorldUpdater = gustave::worlds::sync::detail::WorldUpdater<libCfg>;

using ContactIndex = Contacts::ContactIndex;
using Direction = ContactIndex::Direction;
using Solver = WorldData::Solver;
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

TEST_CASE("worlds::sync::Contacts") {
    WorldData world = makeWorld();

    Transaction t;
    t.addBlock({ {2,2,2}, concrete_20m, blockMass, false });
    t.addBlock({ {2,1,2}, concrete_20m, blockMass, true });
    WorldUpdater{ world }.runTransaction(t);

    Contacts contacts{ world };

    SECTION(".at()") {
        SECTION("// valid") {
            auto contact = contacts.at(ContactIndex{ {2,1,2}, Direction::plusY() });
            CHECK_THAT(contact.forceVector(), matchers::WithinRel(blockMass * g, solverPrecision));
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(contacts.at(ContactIndex{ {2,1,2}, Direction::plusX() }), std::out_of_range);
        }
    }

    SECTION(".find()") {
        auto contact = contacts.find(ContactIndex{ {2,1,2}, Direction::plusY() });
        CHECK_THAT(contact.forceVector(), matchers::WithinRel(blockMass * g, solverPrecision));
    }
}
