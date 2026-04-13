/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <Tutorial.hpp>

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

int main(int argc, char** argv) {
    auto tuto = Tutorial{ "World links", argc, argv };
    if (tuto.earlyExitCode()) {
        return *tuto.earlyExitCode();
    }

    // -8<- [start:create-world]
    auto world = newWorld();
    // -8<- [end:create-world]

    // -8<- [start:add-blocks]
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
    // -8<- [end:add-blocks]

    // -8<- [start:direction-alias]
    using Direction = World::ContactIndex::Direction;
    // -8<- [end:direction-alias]


    tuto.section("inspect-contact", "Inspect a specific contact");
    // -8<- [start:inspect-contact]
    {
        auto const contact = world.contacts().at({ {0,4,0}, Direction::plusY() });
        std::cout << "Contact " << contact.index() << ": other block is " << contact.otherBlock().index() << '\n';
    }
    // -8<- [end:inspect-contact]


    tuto.section("contact-status", "Check contact status (valid & solved)");
    // -8<- [start:contact-status]
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
    // -8<- [end:contact-status]


    tuto.section("contact-force", "Contact's force");
    // -8<- [start:contact-force]
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
    // -8<- [end:contact-force]


    tuto.section("link-stress-ratio", "Link's stressRatio");
    // -8<- [start:link-stress-ratio]
    {
        auto printContactStress = [&world](World::ContactIndex const& contactId) -> void {
            auto const cRef = world.contacts().at(contactId);
            std::cout << "Stress of link " << contactId << " = " << cRef.stressRatio() << '\n';
        };
        printContactStress({ {0,4,0}, Direction::plusY() });
        printContactStress({ {0,1,0}, Direction::minusY() });
        printContactStress({ {4,1,0}, Direction::minusY() });
    }
    // -8<- [end:link-stress-ratio]


    tuto.section("structure-stress-ratio", "Stress ratio of a structure");
    // -8<- [start:structure-stress-ratio]
    {
        auto printMaxStressOfStructure = [&world](World::BlockIndex const& blockId) -> void {
            // Gets the StructureReference of a block (unique for a non-foundation).
            auto const structRef = world.blocks().at(blockId).structures().unique();
            std::cout << "Max stress ratio of structure of block " << blockId << " = ";
            // Ensures that we can access forces & stresses of all links in the structure.
            if (structRef.isSolved()) {
                auto result = G::Model::StressRatio{ 0.0, 0.0, 0.0 };
                // Iterate over all links of the structure.
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
    // -8<- [end:structure-stress-ratio]
    tuto.endSection();
}
