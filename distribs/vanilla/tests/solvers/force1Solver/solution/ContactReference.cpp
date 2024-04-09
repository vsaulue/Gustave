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

#include <gustave/solvers/force1Solver/detail/SolutionData.hpp>
#include <gustave/solvers/force1Solver/solution/ContactReference.hpp>

#include <TestHelpers.hpp>

using ContactReference = gustave::solvers::force1Solver::solution::ContactReference<libCfg>;
using SolutionData = gustave::solvers::force1Solver::detail::SolutionData<libCfg>;

using Structure = SolutionData::Basis::Structure;

using ContactIndex = Structure::ContactIndex;
using Link = Structure::Link;
using Node = Structure::Node;
using NodeReference = ContactReference::NodeReference;
using SolutionBasis = SolutionData::Basis;
using SolverConfig = SolutionData::Basis::Config;

TEST_CASE("force1::solutionUtils::ContactReference") {
    static constexpr Real<u.one> precision = 0.001f;
    auto const solverConfig = std::make_shared<SolverConfig const>(g, 1000, precision);

    auto structure = std::make_shared<Structure>();
    structure->addNode(Node{ 5'000.f * u.mass, true });
    structure->addNode(Node{ 15'000.f * u.mass, false });
    structure->addNode(Node{ 7'500.f * u.mass, false });
    structure->addLink(Link{ 0, 1, Normals::y, 2.f * u.area, 1.f * u.length, concrete_20m });
    structure->addLink(Link{ 1, 2, Normals::y, 1.f * u.area, 1.f * u.length, concrete_20m });

    auto const basis = std::make_shared<SolutionBasis>(structure, solverConfig);
    basis->spanPotentials()[0] = 0.f * u.potential;
    basis->spanPotentials()[1] = 0.125f * u.potential;
    basis->spanPotentials()[2] = 0.25f * u.potential;

    SolutionData data{ basis };

    NodeReference n1{ data, 1 };
    NodeReference n2{ data, 2 };

    ContactReference c12{ data, ContactIndex{ 1, true } };
    ContactReference c21{ data, ContactIndex{ 1, false } };

    SECTION(".conductivity()") {
        CHECK(c12.conductivity() == concrete_20m * (1.f * u.length));
    }

    SECTION(".forceCoord()") {
        CHECK(c12.forceCoord() == 2'500'000.f * u.force);
    }

    SECTION(".forceVector()") {
        CHECK(c21.forceVector() == vector3(0.f, 2'500'000.f, 0.f, u.force));
    }

    SECTION(".index()") {
        CHECK(c21.index() == ContactIndex{ 1, false });
    }

    SECTION(".localNode()") {
        SECTION("// isLocal == true") {
            CHECK(c12.localNode() == n1);
        }

        SECTION("// isLocal == false") {
            CHECK(c21.localNode() == n2);
        }
    }

    SECTION(".normal()") {
        SECTION("// isLocal == true") {
            CHECK(c12.normal() == Normals::y);
        }

        SECTION("// isLocal == false") {
            CHECK(c21.normal() == -Normals::y);
        }
    }

    SECTION(".opposite()") {
        CHECK(c12.opposite() == c21);
        CHECK(c21.opposite() == c12);
    }

    SECTION(".otherNode()") {
        SECTION("// isLocal == true") {
            CHECK(c12.otherNode() == n2);
        }

        SECTION("// isLocal == false") {
            CHECK(c21.otherNode() == n1);
        }
    }
}
