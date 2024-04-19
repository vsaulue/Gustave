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

#include <gustave/vanilla/Gustave.hpp>

using G = gustave::vanilla::Gustave;

static constexpr auto u = G::units();

static auto const blockDimensions = G::vector3(1.f, 1.f, 1.f, u.length);
static auto const gravity = G::vector3(0.f, -10.f, 0.f, u.acceleration);
static auto const solverPrecision = 0.001f;

G::Worlds::SyncWorld newWorld() {
    using Solver = G::Worlds::SyncWorld::Solver;
    auto solver = Solver{ Solver::Config{ gravity, solverPrecision } };
    return G::Worlds::SyncWorld{ blockDimensions, std::move(solver) };
}

int main() {
    auto const concrete_20m = G::Worlds::SyncWorld::BlockReference::PressureStress{
        20'000'000.f * u.pressure, // max compressive pressure
        14'000'000.f * u.pressure, // max shear pressure
        2'000'000.f * u.pressure, // max tensile pressure
    };
    G::Real<u.mass> const blockMass = 2'400.f * u.mass;

    auto world = newWorld();
    {
        auto transaction = G::Worlds::SyncWorld::Transaction{};
        transaction.addBlock({ {0,1,0}, concrete_20m, blockMass, false });
        transaction.addBlock({ {0,0,0}, concrete_20m, blockMass, true });
        world.modify(transaction);
    }

    auto const contactIndex = G::Worlds::SyncWorld::ContactIndex{ {0,0,0}, G::Worlds::SyncWorld::ContactIndex::Direction::plusY() };
    auto const contactForce = world.contacts().at(contactIndex).forceVector();
    auto const expectedForce = blockMass * gravity;

    if ((contactForce - expectedForce).norm() <= solverPrecision * expectedForce.norm()) {
        return 0;
    } else {
        return 1;
    }
}
