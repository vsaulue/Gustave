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

#include <TestHelpers.hpp>

#include <gustave/core/solvers/force1Solver/detail/LayerDecomposition.hpp>

using LayerDecomposition = gustave::core::solvers::force1Solver::detail::LayerDecomposition<libCfg>;

using DecLayer = LayerDecomposition::DecLayer;
using F1Structure = LayerDecomposition::F1Structure;
using LayerIndex = LayerDecomposition::LayerIndex;

using Config = F1Structure::Config;
using Structure = F1Structure::Structure;

using Conductivity = Structure::Link::Conductivity;
using NodeIndex = Structure::NodeIndex;

TEST_CASE("core::force1Solver::detail::LayerDecomposition") {
    static constexpr Real<u.mass> blockMass = 1000.f * u.mass;
    Real<u.force> const blockWeight = blockMass * g.norm();
    Conductivity const conductivity{ 1000.f * u.conductivity, 200.f * u.conductivity, 100.f * u.conductivity };

    auto const config = Config{ g, 0.001f };
    auto structure = Structure{};

    auto addNode = [&](bool isFoundation) -> NodeIndex {
        return structure.addNode(Structure::Node{ blockMass, isFoundation });
    };

    addNode(false); // unreachable

    // x == 2
    NodeIndex const x2y0 = addNode(true);
    NodeIndex const x2y1 = addNode(false);
    NodeIndex const x2y2 = addNode(false);
    NodeIndex const x2y3 = addNode(false);
    NodeIndex const x2y4 = addNode(false);
    // x == 3
    NodeIndex const x3y1 = addNode(false);
    NodeIndex const x3y3 = addNode(false);
    // x == 4
    NodeIndex const x4y0 = addNode(true);
    NodeIndex const x4y1 = addNode(false);
    NodeIndex const x4y2 = addNode(false);
    NodeIndex const x4y3 = addNode(false);

    auto addLink = [&](NodeIndex localId, NodeIndex otherId, NormalizedVector3 const& normal) {
        structure.addLink(Structure::Link{ localId, otherId, normal, conductivity });
    };

    // x == 2
    addLink(x2y0, x2y1, Normals::y);
    addLink(x2y1, x2y2, Normals::y);
    addLink(x2y2, x2y3, Normals::y);
    addLink(x2y3, x2y4, Normals::y);
    // x == 4
    addLink(x4y3, x4y2, -Normals::y);
    addLink(x4y2, x4y1, -Normals::y);
    addLink(x4y1, x4y0, -Normals::y);
    // y == 1
    addLink(x2y1, x3y1, Normals::x);
    addLink(x3y1, x4y1, Normals::x);
    // y == 3
    addLink(x4y3, x3y3, -Normals::x);
    addLink(x3y3, x2y3, -Normals::x);

    auto const fStructure = F1Structure{ structure, config };
    auto const layerDecomposition = LayerDecomposition{ fStructure };

    SECTION(".decLayers") {
        auto const expected = std::vector<DecLayer>{
            {{x2y4}, blockWeight, 2},
            {{x3y3}, blockWeight, 2},
            {{x2y3, x4y3}, 4.f * blockWeight, 3},
            {{x2y2, x4y2}, 6.f * blockWeight, 5},
            {{x3y1}, blockWeight, 5},
            {{x2y1, x4y1}, 9.f * blockWeight, 6},
            {{x2y0, x4y0}, 11.f * blockWeight, 0},
        };
        CHECK_THAT(layerDecomposition.decLayers, matchers::c2::RangeEquals(expected));
    }

    SECTION(".layerOfNode") {
        auto const expected = std::vector<LayerIndex>{ 0,6,5,3,2,0,4,1,6,5,3,2 };
        CHECK_THAT(layerDecomposition.layerOfNode, matchers::c2::RangeEquals(expected));
    }

    SECTION(".reachedCount") {
        CHECK(layerDecomposition.reachedCount == 11);
    }
}
