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
#include <memory>

// -8<- [start:distrib-unitless]
// Choosing the Std Unitless distribution, with double precision
#include <gustave/distribs/std/unitless/Gustave.hpp>

using G = gustave::distribs::std::unitless::Gustave<double>;
// -8<- [end:distrib-unitless]

// -8<- [start:type-aliases]
using Structure = G::Solvers::Structure;
using Solver = G::Solvers::F1Solver;
// -8<- [end:type-aliases]

// -8<- [start:newSolver]
[[nodiscard]]
static Solver newSolver() {
    auto const g = G::vector3(0.f, -10.f, 0.f); // gravity acceleration (metre/second²).
    auto const solverPrecision = 0.01; // precision of the force balancer (here 1%).
    return Solver{ Solver::Config{ g, solverPrecision } };
}
// -8<- [end:newSolver]

int main() {
    std::cout << "\n\n--------------------\n";
    std::cout << "Step 1: New empty solver structure\n";

    // -8<- [start:new-structure]
    auto structure = std::make_shared<Structure>();
    std::cout << "Structure of " << structure->nodes().size() << " blocks\n";
    std::cout << "Structure of " << structure->links().size() << " links\n";
    // -8<- [end:new-structure]



    std::cout << "\n\n--------------------\n";
    std::cout << "Step 2: Add nodes (= blocks)\n";

    // -8<- [start:add-blocks]
    auto const blockMass = 3'000.0; // kilogram
    //          xy
    auto const n00 = structure->addNode(Structure::Node{ blockMass, true });
    auto const n01 = structure->addNode(Structure::Node{ blockMass, false });
    auto const n02 = structure->addNode(Structure::Node{ blockMass, false });
    auto const n12 = structure->addNode(Structure::Node{ blockMass, false });
    auto const n22 = structure->addNode(Structure::Node{ blockMass, false });
    auto const n21 = structure->addNode(Structure::Node{ blockMass, false });
    auto const n20 = structure->addNode(Structure::Node{ blockMass, true });
    std::cout << "Structure of " << structure->nodes().size() << " blocks\n";
    std::cout << "Structure of " << structure->links().size() << " links\n";
    // -8<- [end:add-blocks]



    std::cout << "\n\n--------------------\n";
    std::cout << "Step 3: Add links\n";

    // -8<- [start:add-links]
    // { compression, shear, tensile } in Newton/metre
    auto const wallConductivity = G::Model::ConductivityStress{ 1'000'000.0, 500'000.0, 200'000.0 };
    auto const roofConductivity = G::Model::ConductivityStress{ 100'000.0, 500'000.0, 100'000.0 };

    auto const plusY = G::NormalizedVector3{ 0.0, 1.0, 0.0 };
    auto const plusX = G::NormalizedVector3{ 1.0, 0.0, 0.0 };

    // left wall
    auto const l00_01 = structure->addLink(Structure::Link{ n00, n01, plusY, wallConductivity });
    structure->addLink(Structure::Link{ n01, n02, plusY, wallConductivity });
    // right wall
    auto const l20_21 = structure->addLink(Structure::Link{ n20, n21, plusY, wallConductivity });
    structure->addLink(Structure::Link{ n21, n22, plusY, wallConductivity });
    // roof
    structure->addLink(Structure::Link{ n02, n12, plusX, roofConductivity });
    structure->addLink(Structure::Link{ n12, n22, plusX, roofConductivity });

    std::cout << "Structure of " << structure->nodes().size() << " blocks\n";
    std::cout << "Structure of " << structure->links().size() << " links\n";
    // -8<- [end:add-links]



    std::cout << "\n\n--------------------\n";
    std::cout << "Step 4: Configure a solver\n";

    // -8<- [start:configure-solver]
    auto const solver = newSolver();

    std::cout << "Solver gravity vector = " << solver.config().g() << '\n';
    std::cout << "Solver target max error = " << solver.config().targetMaxError() << '\n';
    // -8<- [end:configure-solver]



    std::cout << "\n\n--------------------\n";
    std::cout << "Step 5: Solve a structure\n";

    // -8<- [start:solve-structure]
    auto const solverResult = solver.run(structure);
    std::cout << "solution.isSolved() = " << solverResult.isSolved() << '\n';
    // -8<- [end:solve-structure]



    std::cout << "\n\n--------------------\n";
    std::cout << "Step 6: Inspect a solution's forces\n";

    // -8<- [start:inspect-solution]
    auto const& solution = solverResult.solution();
    std::cout << "Force vector on block 00 by 01 = " << solution.contacts().at({l00_01, true}).forceVector() << '\n';
    std::cout << "Force vector on block 21 by 22 = " << solution.contacts().at({l20_21, false}).forceVector() << '\n';
    // -8<- [end:inspect-solution]
}
