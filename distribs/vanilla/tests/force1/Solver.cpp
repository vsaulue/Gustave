/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

using Solver = Gustave::Solvers::Force1Solver<cfg>;

using Link = Solver::Structure::Link;
using Node = Solver::Structure::Node;
using Solution = Solver::Solution;
using Structure = Solver::Structure;

TEST_CASE("Force1::Solver") {
    SECTION("// pillar") {
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
        constexpr float precision = 0.001f;
        auto structure = std::make_shared<Structure const>(makePillar(blockCount));
        auto const solver = Solver{ std::make_shared<Solver::Config const>(g, 1000, precision) };
        auto const result = solver.run(structure);
        Solution const& solution = result.solution();
        CHECK_THAT(solution.forceVector(0, 1), M::WithinRel(float(blockCount - 1) * blockMass * g, precision));
        CHECK_THAT(solution.forceVector(1, 2), M::WithinRel(float(blockCount - 2) * blockMass * g, precision));
        CHECK_THAT(solution.forceVector(2, 3), M::WithinRel(float(blockCount - 3) * blockMass * g, precision));
    }
}
