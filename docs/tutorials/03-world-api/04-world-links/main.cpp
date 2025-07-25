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

    // { compression, shear, tensile } in pascal
    auto const strongBlockStress = G::Model::PressureStress{ 500'000.0, 500'000.0, 500'000.0 };
    auto const weakBlockStress = G::Model::PressureStress{ 100'000.0, 100'000.0, 100'000.0 };

    // kilogram
    auto const mass = 3'000.0;
    {
        auto tr = World::Transaction{};
        // The chair
        tr.addBlock({ { 0,8,0 }, weakBlockStress, mass, false });
        tr.addBlock({ { 0,7,0 }, weakBlockStress, mass, false });
        tr.addBlock({ { 0,6,0 }, weakBlockStress, mass, false });
        tr.addBlock({ { 0,5,0 }, weakBlockStress, mass, false });
        tr.addBlock({ { 0,4,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 0,3,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 0,2,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 0,1,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 0,0,0 }, strongBlockStress, mass, true });

        tr.addBlock({ { 1,4,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 2,4,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 3,4,0 }, strongBlockStress, mass, false });

        tr.addBlock({ { 4,4,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 4,3,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 4,2,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 4,1,0 }, strongBlockStress, mass, false });
        tr.addBlock({ { 4,0,0 }, strongBlockStress, mass, true });

        // Floating blocks
        tr.addBlock({ { 3,8,0 }, weakBlockStress, mass, false });
        tr.addBlock({ { 4,8,0 }, weakBlockStress, mass, false });

        world.modify(tr);
    }

    using Direction = World::ContactIndex::Direction;

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 1: Inspect a specific contact\n";
    {
        auto const contactRef = world.contacts().at({ {0,4,0}, Direction::plusY() });
        std::cout << "Contact " << contactRef.index() << ": other block is " << contactRef.otherBlock().index() << '\n';
    }

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 2: Check contact status (valid & solved)\n";
    {
        auto printContactStatus = [&world](World::ContactIndex const& contactId) -> void {
            auto const contactRef = world.contacts().find(contactId);
            std::cout << "Contact " << contactId << ": ";
            if (contactRef.isValid()) {
                if (contactRef.isSolved()) {
                    std::cout << "solved\n";
                } else {
                    std::cout << "unsolved\n";
                }
            } else {
                std::cout << "invalid\n";
            }
        };
        printContactStatus({ {0,4,0}, Direction::plusY() });
        printContactStatus({ {3,8,0}, Direction::plusX() });
        printContactStatus({ {9,9,0}, Direction::minusX() });
    }

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 3: Contact's force\n";
    {
        auto printContactForce = [&world](World::ContactIndex const& contactId) -> void {
            auto const cRef = world.contacts().at(contactId);
            std::cout << "Force vector by block " << cRef.otherBlock().index() << " on block " << cRef.localBlock().index();
            std::cout << " = " << cRef.forceVector() << '\n';
        };
        printContactForce({ {0,4,0}, Direction::plusY() });
        printContactForce({ {0,1,0}, Direction::minusY() });
        printContactForce({ {4,1,0}, Direction::minusY() });
    }

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 4: Link's stressRatio\n";
    {
        auto printContactStress = [&world](World::ContactIndex const& contactId) -> void {
            auto const cRef = world.contacts().at(contactId);
            std::cout << "Stress of link " << contactId << " = " << cRef.stressRatio() << '\n';
        };
        printContactStress({ {0,4,0}, Direction::plusY() });
        printContactStress({ {0,1,0}, Direction::minusY() });
        printContactStress({ {4,1,0}, Direction::minusY() });
    }

    std::cout << "\n\n--------------------\n";
    std::cout << "Step 5: Stress ratio of a structure\n";
    {
        auto printMaxStressOfStructure = [&world](World::BlockIndex const& blockId) -> void {
            auto const structRef = world.blocks().at(blockId).structures()[0];
            std::cout << "Max stress ratio of structure of block " << blockId << " = ";
            if (structRef.isSolved()) {
                auto result = G::Model::StressRatio{ 0.0, 0.0, 0.0 };
                for (auto const& linkRef : structRef.links()) {
                    result.mergeMax(linkRef.stressRatio());
                }
                std::cout << result << '\n';
            } else {
                std::cout << "unsolved structure\n";
            }
        };
        printMaxStressOfStructure({ 0,1,0 });
        printMaxStressOfStructure({ 3,8,0 });
    }
}
