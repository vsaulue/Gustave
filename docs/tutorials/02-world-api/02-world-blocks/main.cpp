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


// Choosing the Std Unitless distribution, with double precision
#include <gustave/distribs/std/unitless/Gustave.hpp>

using G = gustave::distribs::std::unitless::Gustave<double>;

using World = G::Worlds::SyncWorld;

[[nodiscard]]
static World newWorld() {
    auto g = G::vector3(0.f, -10.f, 0.f); // gravity acceleration (metre/second²).
    auto solverPrecision = 0.01; // precision of the force balancer (here 1%).
    auto blockSize = G::vector3(1.f, 1.f, 1.f); // block dimension (cube with 1m edge).

    auto solverConfig = World::Solver::Config{ g, solverPrecision };
    return World{ blockSize, World::Solver{ solverConfig } };
}

int main() {
    auto world = newWorld();

    // Helper function to list blocks.
    auto printBlocks = [&world]() -> void {
        auto const& blocks = world.blocks();
        std::cout << "List of blocks (size = " << blocks.size() << "):\n";
        for (auto const& block : blocks) {
            std::cout << "- " << block.index() << ": mass = " << block.mass() << ", isFoundation = " << block.isFoundation() << '\n';
        }
    };

    std::cout << "Step 1: create an empty world\n";
    printBlocks();

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 2: add blocks\n";
    auto const maxBlockStress = G::Model::PressureStress{
        100'000.0, // compression (Pascal)
        50'000.0,  // shear (Pascal)
        20'000.0,  // tensile (Pascal)
    };
    auto const heavyMass = 10'000.0;  // kilogram
    auto const lightMass = 1'000.0;   // kilogram
    {
        auto tr = World::Transaction{};
        tr.addBlock({ { 0,0,0 }, maxBlockStress, heavyMass, true }); // foundation block at coordinates {0,0,0}.
        for (int i = 1; i <= 6; ++i) {
            tr.addBlock({ { 0,i,0 }, maxBlockStress, lightMass, false }); // non-foundation block at coordinates {0,i,0}.
        }
        world.modify(tr);
    }
    printBlocks();

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 3: delete blocks\n";
    {
        auto tr = World::Transaction{};
        tr.removeBlock({ 0,6,0 });
        tr.removeBlock({ 0,5,0 });
        world.modify(tr);
    }
    printBlocks();

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 4: modify the top block\n";
    {
        auto tr = World::Transaction{};
        tr.removeBlock({ 0,4,0 });
        tr.addBlock({ {0,4,0}, maxBlockStress, heavyMass, false });
        world.modify(tr);
    }
    printBlocks();

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 5: inspect a block\n";
    auto inspectBlock = [&world](World::BlockIndex const& blockId) -> void {
        std::cout << "Block at " << blockId << ": ";
        auto const blockRef = world.blocks().find(blockId);
        if (blockRef.isValid()) {
            std::cout << "mass = " << blockRef.mass() << ", isFoundation = " << blockRef.isFoundation();
        } else {
            std::cout << "invalid";
        }
        std::cout << '\n';
    };
    inspectBlock({ 0,0,0 });
    inspectBlock({ 0,1,0 });
    inspectBlock({ 9,9,9 });
}
