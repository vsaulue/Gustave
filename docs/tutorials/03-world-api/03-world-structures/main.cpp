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
using Solver = World::Solver;

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

int main() {
    // -8<- [start:create-world]
    auto world = newWorld();
    // -8<- [end:create-world]

    // -8<- [start:add-blocks]
    // kilogram
    auto const foundationMass = 10'0000.0;
    auto const struct1Mass = 1'000.0;
    auto const struct2Mass = 2'000.0;
    auto const struct3Mass = 3'000.0;
    // { compression, shear, tensile } in pascal
    auto const maxBlockStress = G::Model::PressureStress{ 100'000.0, 50'000.0, 20'000.0 };
    {
        auto tr = World::Transaction{};
        // foundation shared between structure 1 & 2
        tr.addBlock({ { 0,0,0 }, maxBlockStress, foundationMass, true });
        // structure 1
        tr.addBlock({ { 0,1,0 }, maxBlockStress, struct1Mass, false });
        tr.addBlock({ { 0,2,0 }, maxBlockStress, struct1Mass, false });
        // structure 2
        tr.addBlock({ { 1,0,0 }, maxBlockStress, struct2Mass, false });
        tr.addBlock({ { 2,0,0 }, maxBlockStress, struct2Mass, false });
        tr.addBlock({ { 3,0,0 }, maxBlockStress, struct2Mass, false });
        tr.addBlock({ { 4,0,0 }, maxBlockStress, struct2Mass, true });
        // structure 3 (no foundations)
        tr.addBlock({ { 7,0,0 }, maxBlockStress, struct3Mass, false });
        tr.addBlock({ { 8,0,0 }, maxBlockStress, struct3Mass, false });

        world.modify(tr);
    }
    // -8<- [end:add-blocks]

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 1: list all structures and their blocks\n";

    // -8<- [start:list-world-structures]
    std::cout << "List of structures (size = " << world.structures().size() << ")\n";
    for (auto const& structure : world.structures()) {
        std::cout << "- structure of " << structure.blocks().size() << " blocks:\n";
        for (auto const& block : structure.blocks()) {
            std::cout << "  - " << block.index() << '\n';
        }
    }
    // -8<- [end:list-world-structures]

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 2: list the structures of a block\n";
    // -8<- [start:list-block-structures]
    auto listStructuresOfBlock = [&world](World::BlockIndex const& blockId) -> void {
        auto const blockRef = world.blocks().at(blockId);
        std::cout << "Structures of block " << blockId << " (size = " << blockRef.structures().size() << "):\n";
        for (auto const& structureRef : blockRef.structures()) {
            std::cout << "- structure of " << structureRef.blocks().size() << " blocks\n";
        }
    };
    listStructuresOfBlock({ 0,0,0 });
    listStructuresOfBlock({ 7,0,0 });
    // -8<- [end:list-block-structures]

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 3: structure status (valid, solved)\n";
    // -8<- [start:structure-status]
    auto printStructureStatusOfBlock = [&world](World::BlockIndex const& blockId) -> void {
        auto const structureRef = world.blocks().at(blockId).structures()[0];
        std::cout << "Statut of structure of block " << blockId << ": ";
        if (structureRef.isValid()) {
            if (structureRef.isSolved()) {
                std::cout << "solved\n";
            } else {
                std::cout << "not solved\n";
            }
        } else {
            std::cout << "invalid\n";
        }
    };
    printStructureStatusOfBlock({ 0,1,0 });
    printStructureStatusOfBlock({ 7,0,0 });
    // -8<- [end:structure-status]
}
