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
#include <gustave/solvers/force1Solver/solution/NodeReference.hpp>

#include <TestHelpers.hpp>

using NodeReference = gustave::solvers::force1Solver::solution::NodeReference<libCfg>;
using SolutionData = gustave::solvers::force1Solver::detail::SolutionData<libCfg>;

using Structure = SolutionData::Basis::Structure;

using ContactIndex = Structure::ContactIndex;
using ContactReference = NodeReference::ContactReference;
using Link = Structure::Link;
using LinkIndex = Structure::LinkIndex;
using Node = Structure::Node;
using SolutionBasis = SolutionData::Basis;
using SolverConfig = SolutionData::Basis::Config;

TEST_CASE("force1::solutionUtils::NodeReference") {
    static constexpr Real<u.one> precision = 0.001f;
    auto const solverConfig = std::make_shared<SolverConfig const>(g, 1000, precision);

    auto structure = std::make_shared<Structure>();
    structure->addNode(Node{ 5'000.f * u.mass, true });
    structure->addNode(Node{ 10'000.f * u.mass, false });
    structure->addNode(Node{ 15'000.f * u.mass, false });
    structure->addLink(Link{ 0, 1, Normals::y, 2.f * u.area, 1.f * u.length, concrete_20m });
    structure->addLink(Link{ 1, 2, Normals::y, 1.f * u.area, 1.f * u.length, concrete_20m });

    auto const basis = std::make_shared<SolutionBasis>(structure, solverConfig);
    basis->spanPotentials()[0] = 0.f * u.potential;
    basis->spanPotentials()[1] = 0.125f * u.potential;
    basis->spanPotentials()[2] = 0.25f * u.potential;

    SolutionData data{ basis };

    NodeReference n0{ data, 0 };
    NodeReference n1{ data, 1 };
    NodeReference n2{ data, 2 };

    auto makeContactRef = [&data](LinkIndex linkId, bool isOnLocalNode) {
        return ContactReference{ data, ContactIndex{ linkId, isOnLocalNode } };
    };

    SECTION(".contacts()") {
        SECTION(".at()") {
            SECTION("// valid") {
                ContactIndex id{ 1, false };
                ContactReference cRef = n2.contacts().at(id);
                CHECK(cRef == ContactReference{ data, id });
            }

            SECTION("// invalid") {
                SECTION("(link index too great)") {
                    CHECK_THROWS_AS(n2.contacts().at(ContactIndex{ 2, true }), std::out_of_range);
                }

                SECTION("(wrong link index)") {
                    CHECK_THROWS_AS(n2.contacts().at(ContactIndex{ 0, true }), std::out_of_range);
                }

                SECTION("(wrong isOnLocalNode)") {
                    CHECK_THROWS_AS(n2.contacts().at(ContactIndex{ 1, true }), std::out_of_range);
                }
            }
        }

        SECTION(".begin() // & .end()") {
            SECTION("// Node 2") {
                std::vector<ContactReference> const expected = {
                    makeContactRef( 1, false),
                };
                CHECK_THAT(n2.contacts(), matchers::c2::UnorderedRangeEquals(expected));
            }

            SECTION("// Node 1") {
                std::vector<ContactReference> const expected = {
                    makeContactRef(0, false),
                    makeContactRef(1, true),
                };
                CHECK_THAT(n1.contacts(), matchers::c2::UnorderedRangeEquals(expected));
            }
        }

        SECTION(".size()") {
            CHECK(n1.contacts().size() == 2);
        }
    }

    SECTION(".forceVectorFrom(NodeIndex)") {
        CHECK(n0.forceVectorFrom(1) == vector3(0.f, -5'000'000.f, 0.f, u.force));
    }

    SECTION(".index()") {
        CHECK(n0.index() == 0);
        CHECK(n1.index() == 1);
    }

    SECTION(".isFoundation()") {
        CHECK(n0.isFoundation());
        CHECK_FALSE(n1.isFoundation());
    }

    SECTION(".mass()") {
        CHECK(n0.mass() == 5'000.f * u.mass);
    }

    SECTION(".netForceCoord()") {
        CHECK(n1.netForceCoord() == -2'400'000.f * u.force);
    }

    SECTION(".netForceVector()") {
        CHECK(n1.netForceVector() == vector3(0.f, 2'400'000.f, 0.f, u.force));
    }

    SECTION(".potential()") {
        CHECK(n1.potential() == 0.125f * u.potential);
    }

    SECTION(".relativeError()") {
        CHECK(n1.relativeError() == 24.f);
    }

    SECTION(".weight()") {
        CHECK(n0.weight() == 50'000.f * u.force);
    }

    SECTION(".weightVector()") {
        CHECK(n0.weightVector() == vector3(0.f, -50'000.f, 0.f, u.force));
    }
}
