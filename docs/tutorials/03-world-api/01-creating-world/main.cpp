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

#include <iostream>

// -8<- [start:distrib-unitless]
// Choosing the Std Unitless distribution, with double precision
#include <gustave/distribs/std/unitless/Gustave.hpp>

using G = gustave::distribs::std::unitless::Gustave<double>;
// -8<- [end:distrib-unitless]

// -8<- [start:type-aliases]
using World = G::Worlds::SyncWorld;
using Solver = World::Solver;
// -8<- [end:type-aliases]

// -8<- [start:newWorld]
[[nodiscard]]
static Solver newSolver() {
    auto const g = G::vector3(0.f, -10.f, 0.f); // gravity acceleration (metre/second²).
    auto const solverPrecision = 0.01; // precision of the force balancer (here 1%).
    return Solver{ Solver::Config{ g, solverPrecision } };
}

[[nodiscard]]
static World newWorld() {
    auto blockSize = G::vector3(1.f, 1.f, 1.f); // block dimension (cube with 1m edge).
    return World{ blockSize, newSolver() };
}
// -8<- [end:newWorld]

int main() {
    // -8<- [start:newWorld-usage]
    auto world = newWorld();

    std::cout << "Tutorial: creating a new SyncWorld.\n\n";

    std::cout << "- number of blocks = " << world.blocks().size() << '\n';
    std::cout << "- number of structures = " << world.structures().size() << '\n';
    // -8<- [end:newWorld-usage]
}
