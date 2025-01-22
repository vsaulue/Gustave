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

#include <TestHelpers.hpp>

#include <gustave/core/solvers/force1Solver/detail/ClusterStructure.hpp>

using ClusterStructure = gustave::core::solvers::force1Solver::detail::ClusterStructure<libCfg>;

using Cluster = ClusterStructure::Cluster;
using ClusterContact = ClusterStructure::ClusterContact;
using ClusterIndex = ClusterStructure::ClusterIndex;
using F1Structure = ClusterStructure::F1Structure;

using Config = F1Structure::Config;
using F1BasicContact = ClusterContact::F1BasicContact;
using Structure = F1Structure::Structure;

using Conductivity = Structure::Link::Conductivity;
using NodeIndex = Structure::NodeIndex;

TEST_CASE("core::force1Solver::detail::ClusterStructure") {
    static constexpr Real<u.mass> blockMass = 1000.f * u.mass;
    Conductivity const conductivity{ 1000.f * u.conductivity, 200.f * u.conductivity, 100.f * u.conductivity };
    auto const blockWeight = g.norm() * blockMass;

    auto const config = Config{ g, 0.001f };
    auto structure = Structure{};

    auto addNode = [&](bool isFoundation) -> NodeIndex {
        return structure.addNode(Structure::Node{ blockMass, isFoundation });
    };

    auto addLink = [&](NodeIndex localId, NodeIndex otherId, NormalizedVector3 const& normal) {
        structure.addLink(Structure::Link{ localId, otherId, normal, conductivity });
    };

    SECTION("{ width = 1 }") {
        // Nodes
        // y == 0
        NodeIndex const x2y0 = addNode(true);
        NodeIndex const x3y0 = addNode(true);
        NodeIndex const x5y0 = addNode(true);
        NodeIndex const x6y0 = addNode(false);
        // y == 1
        NodeIndex const x1y1 = addNode(false);
        NodeIndex const x2y1 = addNode(false);
        NodeIndex const x3y1 = addNode(false);
        NodeIndex const x4y1 = addNode(false);
        NodeIndex const x5y1 = addNode(false);
        // x == 8
        NodeIndex const x8y0 = addNode(true);
        NodeIndex const x8y1 = addNode(false);
        NodeIndex const x8y2 = addNode(false);
        // y == 2
        NodeIndex const x2y2 = addNode(false);
        NodeIndex const x3y2 = addNode(false);
        NodeIndex const x4y2 = addNode(false);
        NodeIndex const x5y2 = addNode(false);
        NodeIndex const x6y2 = addNode(false);
        // y == 3
        NodeIndex const x2y3 = addNode(false);
        NodeIndex const x5y3 = addNode(false);
        NodeIndex const x6y3 = addNode(false);

        // Links
        // y == 0
        addLink(x5y0, x6y0, Normals::x);
        // y == 1
        addLink(x1y1, x2y1, Normals::x);
        addLink(x2y1, x3y1, Normals::x);
        addLink(x3y1, x4y1, Normals::x);
        addLink(x4y1, x5y1, Normals::x);
        // y == 2
        addLink(x6y2, x5y2, -Normals::x);
        addLink(x5y2, x4y2, -Normals::x);
        addLink(x4y2, x3y2, -Normals::x);
        addLink(x3y2, x2y2, -Normals::x);
        // y == 3
        addLink(x5y3, x6y3, Normals::x);
        // x == 2
        addLink(x2y0, x2y1, Normals::y);
        addLink(x2y1, x2y2, Normals::y);
        addLink(x2y2, x2y3, Normals::y);
        // x == 3
        addLink(x3y2, x3y1, -Normals::y);
        addLink(x3y1, x3y0, -Normals::y);
        // x == 4
        addLink(x4y1, x4y2, Normals::y);
        // x == 5
        addLink(x5y3, x5y2, -Normals::y);
        addLink(x5y2, x5y1, -Normals::y);
        addLink(x5y1, x5y0, -Normals::y);
        // x == 6
        addLink(x6y2, x6y3, Normals::y);
        // x == 8
        addLink(x8y0, x8y1, Normals::y);
        addLink(x8y1, x8y2, Normals::y);

        auto const fStructure = F1Structure{ structure, config };
        auto const cStructure = ClusterStructure{ fStructure };

        SECTION(".clusters()") {
            std::vector<Cluster> const expected = {
                Cluster{ {0,5}, 5.f * blockWeight },
                Cluster{ {5,6}, 4.f * blockWeight },
                Cluster{ {11,2}, 4.f * blockWeight },
            };
            CHECK_THAT(cStructure.clusters(), matchers::c2::RangeEquals(expected));
        }

        SECTION(".clusterOfNode()") {
            static constexpr auto inv = ClusterStructure::invalidClusterId();
            std::vector<ClusterIndex> const expected = {
                inv, inv, inv, inv, // y == 0
                0, 0, 0, 1, 1,      // y == 1
                inv, inv, inv,      // x == 8
                0, 1, 1, 2, 2,      // y == 2
                0, 2, 2,            // y == 3
            };
            CHECK_THAT(cStructure.clusterOfNode(), matchers::c2::RangeEquals(expected));
        }

        SECTION(".contacts()") {
            auto lowerContact = [&](NodeIndex localId, NodeIndex otherId) {
                return ClusterContact{ F1BasicContact{ otherId, conductivity.tensile(), conductivity.compression()}, localId };
            };
            auto sideContact = [&](NodeIndex localId, NodeIndex otherId) {
                return ClusterContact{ F1BasicContact{ otherId, conductivity.shear(), conductivity.shear()}, localId };
            };
            auto upperContact = [&](NodeIndex localId, NodeIndex otherId) {
                return ClusterContact{ F1BasicContact{ otherId, conductivity.compression(), conductivity.tensile()}, localId };
            };

            std::vector<ClusterContact> const expected = {
                lowerContact(x2y1, x2y0),
                sideContact(x3y1, x4y1),
                upperContact(x3y1, x3y2),
                lowerContact(x3y1, x3y0),
                sideContact(x2y2, x3y2),
                sideContact(x4y1, x3y1),
                upperContact(x5y1, x5y2),
                lowerContact(x5y1, x5y0),
                sideContact(x4y2, x5y2),
                sideContact(x3y2, x2y2),
                lowerContact(x3y2, x3y1),
                sideContact(x5y2, x4y2),
                lowerContact(x5y2, x5y1),
            };
            CHECK_THAT(cStructure.contacts(), matchers::c2::RangeEquals(expected));
        }

        SECTION(".contactsOf(NodeIndex)") {
            auto const expected = std::span<ClusterContact const>{ &cStructure.contacts()[5], 6 };
            auto const result = cStructure.contactsOf(1);
            CHECK(result.data() == expected.data());
            CHECK(result.size() == expected.size());
        }
    }

    SECTION("{ width = 3 }") {
        // Nodes
        // y == 0
        NodeIndex const x1y0 = addNode(true);
        NodeIndex const x3y0 = addNode(true);
        NodeIndex const x4y0 = addNode(true);
        NodeIndex const x6y0 = addNode(true);
        // y == 1
        NodeIndex const x1y1 = addNode(false);
        NodeIndex const x3y1 = addNode(false);
        NodeIndex const x4y1 = addNode(false);
        NodeIndex const x6y1 = addNode(false);
        // y == 2
        NodeIndex const x1y2 = addNode(false);
        NodeIndex const x3y2 = addNode(false);
        NodeIndex const x5y2 = addNode(false);
        NodeIndex const x6y2 = addNode(false);
        // y == 3
        NodeIndex const x1y3 = addNode(false);
        NodeIndex const x2y3 = addNode(false);
        NodeIndex const x3y3 = addNode(false);
        NodeIndex const x4y3 = addNode(false);
        NodeIndex const x5y3 = addNode(false);
        NodeIndex const x6y3 = addNode(false);
        // y == 4
        NodeIndex const x3y4 = addNode(false);
        NodeIndex const x5y4 = addNode(false);
        // y == 5
        NodeIndex const x3y5 = addNode(false);
        // y == 6
        NodeIndex const x3y6 = addNode(false);

        // Links
        // y == 1
        addLink(x3y1, x4y1, Normals::x);
        // y == 2
        addLink(x6y2, x5y2, -Normals::x);
        // y == 3
        addLink(x1y3, x2y3, Normals::x);
        addLink(x2y3, x3y3, Normals::x);
        addLink(x3y3, x4y3, Normals::x);
        addLink(x4y3, x5y3, Normals::x);
        addLink(x5y3, x6y3, Normals::x);
        // x == 1
        addLink(x1y0, x1y1, Normals::y);
        addLink(x1y1, x1y2, Normals::y);
        addLink(x1y2, x1y3, Normals::y);
        // x == 3
        addLink(x3y6, x3y5, -Normals::y);
        addLink(x3y5, x3y4, -Normals::y);
        addLink(x3y4, x3y3, -Normals::y);
        addLink(x3y3, x3y2, -Normals::y);
        addLink(x3y2, x3y1, -Normals::y);
        addLink(x3y1, x3y0, -Normals::y);
        // x == 4
        addLink(x4y0, x4y1, Normals::y);
        // x == 5
        addLink(x5y4, x5y3, -Normals::y);
        addLink(x5y3, x5y2, -Normals::y);
        // x == 6
        addLink(x6y0, x6y1, Normals::y);
        addLink(x6y1, x6y2, Normals::y);
        addLink(x6y2, x6y3, Normals::y);

        auto const fStructure = F1Structure{ structure, config };
        auto const cStructure = ClusterStructure{ fStructure, 3 };

        SECTION(".clusters()") {
            std::vector<Cluster> const expected = {
                Cluster{ {0,5}, 14.f * blockWeight },
                Cluster{ {5,3}, 4.f * blockWeight },
            };
            CHECK_THAT(cStructure.clusters(), matchers::c2::RangeEquals(expected));
        }

        SECTION(".clusterOfNode()") {
            static constexpr auto inv = ClusterStructure::invalidClusterId();
            std::vector<ClusterIndex> const expected = {
                inv, inv, inv, inv, // y == 0
                0, 0, 0, 1,         // y == 1
                0, 0, 1, 1,         // y == 2
                0, 0, 0, 0, 0, 1,   // y == 3
                0, 0,               // y == 4
                0,                  // y == 5
                0,                  // y == 6
            };
            CHECK_THAT(cStructure.clusterOfNode(), matchers::c2::RangeEquals(expected));
        }

        SECTION(".contacts()") {
            auto lowerContact = [&](NodeIndex localId, NodeIndex otherId) {
                return ClusterContact{ F1BasicContact{ otherId, conductivity.tensile(), conductivity.compression()}, localId };
            };
            auto sideContact = [&](NodeIndex localId, NodeIndex otherId) {
                return ClusterContact{ F1BasicContact{ otherId, conductivity.shear(), conductivity.shear()}, localId };
            };
            auto upperContact = [&](NodeIndex localId, NodeIndex otherId) {
                return ClusterContact{ F1BasicContact{ otherId, conductivity.compression(), conductivity.tensile()}, localId };
            };

            std::vector<ClusterContact> const expected = {
                lowerContact(x1y1, x1y0),
                sideContact(x5y3, x6y3),
                lowerContact(x5y3, x5y2),
                lowerContact(x3y1, x3y0),
                lowerContact(x4y1, x4y0),
                lowerContact(x6y1, x6y0),
                upperContact(x5y2, x5y3),
                sideContact(x6y3, x5y3),
            };
            CHECK_THAT(cStructure.contacts(), matchers::c2::RangeEquals(expected));
        }

        SECTION(".contactsOf(NodeIndex)") {
            auto const result = cStructure.contactsOf(1);
            CHECK(result.data() == &cStructure.contacts()[5]);
            CHECK(result.size() == 3);
        }
    }
}
