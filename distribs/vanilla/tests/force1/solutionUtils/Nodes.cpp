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
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <gustave/solvers/force1/detail/SolutionData.hpp>
#include <gustave/solvers/force1/solutionUtils/NodeReference.hpp>
#include <gustave/solvers/force1/solutionUtils/Nodes.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/Structure.hpp>

#include <TestHelpers.hpp>

using NodeReference = Gustave::Solvers::Force1::SolutionUtils::NodeReference<cfg>;
using Nodes = Gustave::Solvers::Force1::SolutionUtils::Nodes<cfg>;
using SolutionBasis = Gustave::Solvers::Force1::SolutionBasis<cfg>;
using SolutionData = Gustave::Solvers::Force1::detail::SolutionData<cfg>;
using SolverConfig = Gustave::Solvers::Force1::Config<cfg>;
using Structure = Gustave::Solvers::Structure<cfg>;

using Node = Structure::Node;

static_assert(std::ranges::forward_range<Nodes>);

TEST_CASE("Force1::SolutionUtils::Nodes") {
    static constexpr Real<u.one> precision = 0.001f;
    auto const solverConfig = std::make_shared<SolverConfig const>(g, 1000, precision);

    auto structure = std::make_shared<Structure>();
    structure->addNode(Node{ 5'000.f * u.mass, true });
    structure->addNode(Node{ 10'000.f * u.mass, false });

    auto const basis = std::make_shared<SolutionBasis>(structure, solverConfig);
    basis->spanPotentials()[0] = 0.f * u.potential;
    basis->spanPotentials()[1] = 0.125f * u.potential;

    SolutionData data{ basis };
    Nodes nodes{ data };

    SECTION(".at()") {
        SECTION("// valid") {
            CHECK(nodes.at(1) == NodeReference{ data, 1 });
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(nodes.at(2), std::out_of_range);
        }
    }

    SECTION(".begin() // & .end()") {
        std::vector<NodeReference> const expected = {
            NodeReference{ data, 0 },
            NodeReference{ data, 1 },
        };
        CHECK_THAT(nodes, M::C2::RangeEquals(expected));
    }

    SECTION(".size()") {
        CHECK(nodes.size() == 2);
    }
}
