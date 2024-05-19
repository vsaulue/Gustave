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

#include <TestHelpers.hpp>

#include <gustave/core/solvers/force1Solver/detail/LayerStructure.hpp>

using LayerStructure = gustave::core::solvers::force1Solver::detail::LayerStructure<libCfg>;

using Config = LayerStructure::F1Structure::Config;
using F1BasicContact = LayerStructure::F1BasicContact;
using F1Structure = LayerStructure::F1Structure;
using Layer = LayerStructure::Layer;
using LayerContact = LayerStructure::LayerContact;
using Structure = LayerStructure::F1Structure::Structure;

using Conductivity = Structure::Link::Conductivity;
using NodeIndex = Structure::NodeIndex;

TEST_CASE("core::force1Solver::detail::LayerStructure") {
    static constexpr Real<u.mass> blockMass = 1000.f * u.mass;
    Real<u.force> const blockWeight = blockMass * g.norm();
    Conductivity const conductivity{ 1000.f * u.conductivity, 200.f * u.conductivity, 100.f * u.conductivity };

    auto const config = Config{ g, 0.001f };
    auto structure = Structure{};

    auto addNode = [&](bool isFoundation) -> NodeIndex {
        return structure.addNode(Structure::Node{ blockMass, isFoundation });
    };

    NodeIndex const x0y2 = addNode(false);
    NodeIndex const x1y0 = addNode(true);
    NodeIndex const x1y1 = addNode(false);
    NodeIndex const x1y2 = addNode(false);
    NodeIndex const x2y0 = addNode(true);
    NodeIndex const x2y1 = addNode(false);
    NodeIndex const x2y2 = addNode(false);
    NodeIndex const x3y2 = addNode(false);
    NodeIndex const x4y0 = addNode(true);
    NodeIndex const x4y1 = addNode(false);
    NodeIndex const x4y2 = addNode(false);

    addNode(false); // detached

    auto addLink = [&](NodeIndex localId, NodeIndex otherId, NormalizedVector3 const& normal) {
        structure.addLink(Structure::Link{ localId, otherId, normal, conductivity });
    };

    addLink(x1y0, x1y1, Normals::y);
    addLink(x1y1, x1y2, Normals::y);
    addLink(x0y2, x1y2, Normals::x);
    addLink(x2y0, x2y1, Normals::y);
    addLink(x2y1, x2y2, Normals::y);
    addLink(x1y1, x2y1, Normals::x);
    addLink(x1y2, x2y2, Normals::x);
    addLink(x2y2, x3y2, Normals::x);
    addLink(x4y2, x3y2, -Normals::x);
    addLink(x4y2, x4y1, -Normals::y);
    addLink(x4y1, x4y0, -Normals::y);

    auto const fStructure = F1Structure{ structure, config };
    auto const lStructure = LayerStructure{ fStructure };

    SECTION(".layers()") {
        auto const& layers = lStructure.layers();
        REQUIRE(layers.size() == 3);

        auto addLowerContact = [&](Layer& output, NodeIndex localId, NodeIndex otherId) {
            output.lowContacts.push_back(LayerContact{ F1BasicContact{ otherId, conductivity.tensile(), conductivity.compression()}, localId });
        };
        auto addSideContact = [&](Layer& output, NodeIndex localId, NodeIndex otherId) {
            output.lowContacts.push_back(LayerContact{ F1BasicContact{ otherId, conductivity.shear(), conductivity.shear()}, localId });
        };

        SECTION("// layer 0") {
            Layer expected;
            expected.weight = 8.f * blockWeight;
            expected.nodeIndices.push_back(x1y1);
            addLowerContact(expected, x1y1, x1y0);
            expected.nodeIndices.push_back(x2y1);
            addLowerContact(expected, x2y1, x2y0);
            expected.nodeIndices.push_back(x4y1);
            addLowerContact(expected, x4y1, x4y0);
            CHECK(layers[0] == expected);
        }

        SECTION("// layer 1") {
            Layer expected;
            expected.weight = 5.f * blockWeight;
            expected.nodeIndices.push_back(x1y2);
            addLowerContact(expected, x1y2, x1y1);
            expected.nodeIndices.push_back(x2y2);
            addLowerContact(expected, x2y2, x2y1);
            expected.nodeIndices.push_back(x4y2);
            addLowerContact(expected, x4y2, x4y1);
            CHECK(layers[1] == expected);
        }

        SECTION("// layer 2") {
            Layer expected;
            expected.weight = 2.f * blockWeight;
            expected.nodeIndices.push_back(x0y2);
            addSideContact(expected, x0y2, x1y2);
            expected.nodeIndices.push_back(x3y2);
            addSideContact(expected, x3y2, x2y2);
            addSideContact(expected, x3y2, x4y2);
            CHECK(layers[2] == expected);
        }
    }

    SECTION(".reachedCount()") {
        CHECK(lStructure.reachedCount() == 11);
    }
}
