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

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <gustave/testing/Matchers.hpp>

#include <gustave/solvers/Force1Solver.hpp>

#include <TestHelpers.hpp>

using Solver = gustave::solvers::Force1Solver<libCfg>;

using Link = Solver::Structure::Link;
using Node = Solver::Structure::Node;
using NodeIndex = Solver::Structure::NodeIndex;
using Solution = Solver::Solution;
using Structure = Solver::Structure;

TEST_CASE("force1::Solver") {
    constexpr float precision = 0.001f;
    auto const solver = Solver{ Solver::Config{ g, 1000, precision } };

    SECTION("// solvable: pillar") {
        constexpr Real<u.mass> blockMass = 4000.f * u.mass;
        auto makePillar = [blockMass](unsigned blockCount) {
            Structure structure;
            for (unsigned i = 0; i < blockCount; ++i) {
                structure.addNode(Node{ blockMass, i == 0 });
            }
            for (unsigned i = 0; i < blockCount - 1; ++i) {
                structure.addLink(Link{ i, i + 1,  Normals::y, 1.f * u.area, 1.f * u.length, concrete_20m });
            }
            return structure;
        };
        constexpr unsigned blockCount = 10;
        auto structure = std::make_shared<Structure const>(makePillar(blockCount));
        auto const result = solver.run(structure);
        auto const solvedNodes = result.solution().nodes();
        CHECK_THAT(solvedNodes.at(0).forceVectorFrom(1), matchers::WithinRel(float(blockCount - 1) * blockMass * g, precision));
        CHECK_THAT(solvedNodes.at(1).forceVectorFrom(2), matchers::WithinRel(float(blockCount - 2) * blockMass * g, precision));
        CHECK_THAT(solvedNodes.at(2).forceVectorFrom(3), matchers::WithinRel(float(blockCount - 3) * blockMass * g, precision));
    }

    SECTION("// unsolvable: unreachable non-foundation") {
        auto structure = std::make_shared<Structure>();
        NodeIndex node1 = structure->addNode(Node{ 1000.f * u.mass, true });
        NodeIndex node2 = structure->addNode(Node{ 1000.f * u.mass, false });
        NodeIndex node3 = structure->addNode(Node{ 1000.f * u.mass, false });
        NodeIndex node4 = structure->addNode(Node{ 1000.f * u.mass, false });

        structure->addLink(Link{ node1, node2, Normals::y, 1.f * u.area, 1.f * u.length, concrete_20m });
        structure->addLink(Link{ node3, node4, Normals::y, 1.f * u.area, 1.f * u.length, concrete_20m });
        auto const result = solver.run(structure);
        CHECK_FALSE(result.isSolved());
    }
}
