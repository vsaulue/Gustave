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

#include <gustave/solvers/force1/detail/SolutionData.hpp>
#include <gustave/solvers/force1/solutionUtils/NodeReference.hpp>
#include <gustave/solvers/force1/solutionUtils/ContactReference.hpp>
#include <gustave/solvers/force1/solutionUtils/Contacts.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/Structure.hpp>

#include <TestHelpers.hpp>

using ContactReference = gustave::solvers::force1::solutionUtils::ContactReference<libCfg>;
using Contacts = gustave::solvers::force1::solutionUtils::Contacts<libCfg>;
using NodeReference = gustave::solvers::force1::solutionUtils::NodeReference<libCfg>;
using SolutionBasis = gustave::solvers::force1::SolutionBasis<libCfg>;
using SolutionData = gustave::solvers::force1::detail::SolutionData<libCfg>;
using SolverConfig = gustave::solvers::force1::Config<libCfg>;
using Structure = gustave::solvers::Structure<libCfg>;

using ContactIndex = Structure::ContactIndex;
using Link = Structure::Link;
using Node = Structure::Node;

TEST_CASE("force1::solutionUtils::Contacts") {
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

    ContactReference c12{ data, ContactIndex{ 1, true } };

    Contacts contacts{ data };

    SECTION(".at()") {
        SECTION("// valid") {
            CHECK(contacts.at(ContactIndex{ 1,true }) == c12);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(contacts.at(ContactIndex{2,false}), std::out_of_range);
        }
    }
}
