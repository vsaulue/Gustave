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

#include <gustave/core/solvers/force1Solver/detail/F1Structure.hpp>

using F1Structure = gustave::core::solvers::force1Solver::detail::F1Structure<libCfg>;

using Config = F1Structure::Config;
using F1Contact = F1Structure::F1Contact;
using F1Link = F1Structure::F1Link;
using F1Node = F1Structure::F1Node;
using Structure = F1Structure::Structure;

using Conductivity = Structure::Link::Conductivity;
using NodeIndex = Structure::NodeIndex;

TEST_CASE("core::force1Solver::detail::F1Structure") {
    static constexpr Real<u.mass> blockMass = 1000.f * u.mass;
    Real<u.force> const blockWeight = blockMass * g.norm();
    Conductivity const conductivity{ 1000.f * u.conductivity, 200.f * u.conductivity, 100.f * u.conductivity };

    auto const config = Config{ g, 0.001f };
    auto structure = Structure{};

    auto addNode = [&](bool isFoundation) -> NodeIndex {
        return structure.addNode(Structure::Node{ blockMass, isFoundation });
        };

    addNode(false); // unreachable

    // x == 1
    NodeIndex const x1y1 = addNode(false);
    // x == 2
    NodeIndex const x2y0 = addNode(true);
    NodeIndex const x2y1 = addNode(false);
    NodeIndex const x2y2 = addNode(false);
    NodeIndex const x2y3 = addNode(false);
    // x == 3
    NodeIndex const x3y1 = addNode(false);
    // x == 4
    NodeIndex const x4y1 = addNode(false);

    auto addLink = [&](NodeIndex localId, NodeIndex otherId, NormalizedVector3 const& normal) {
        structure.addLink(Structure::Link{ localId, otherId, normal, conductivity });
        };

    // y == 1
    addLink(x1y1, x2y1, Normals::x);
    addLink(x3y1, x2y1, -Normals::x);
    addLink(x3y1, x4y1, Normals::x);
    // x == 2
    addLink(x2y0, x2y1, Normals::y);
    addLink(x2y1, x2y2, Normals::y);
    addLink(x2y3, x2y2, -Normals::y);

    auto fStructure = F1Structure{ structure, config };

    SECTION(".config()") {
        CHECK(&fStructure.config() == &config);
    }

    SECTION(".g()") {
        CHECK(fStructure.g() == g);
    }

    SECTION(".fContacts()") {
        auto const expected = std::vector<F1Contact>{
            // Node x1y1
            F1Contact{ x2y1, 0, conductivity.shear(), conductivity.shear() },
            // Node x2y0
            F1Contact{ x2y1, 3, conductivity.compression(), conductivity.tensile() },
            // Node x2y1
            F1Contact{ x1y1, 0, conductivity.shear(), conductivity.shear() },
            F1Contact{ x3y1, 1, conductivity.shear(), conductivity.shear() },
            F1Contact{ x2y0, 3, conductivity.tensile(), conductivity.compression() },
            F1Contact{ x2y2, 4, conductivity.compression(), conductivity.tensile() },
            // Node x2y2
            F1Contact{ x2y1, 4, conductivity.tensile(), conductivity.compression() },
            F1Contact{ x2y3, 5, conductivity.compression(), conductivity.tensile() },
            // Node x2y3
            F1Contact{ x2y2, 5, conductivity.tensile(), conductivity.compression() },
            // Node x3y1
            F1Contact{ x2y1, 1, conductivity.shear(), conductivity.shear() },
            F1Contact{ x4y1, 2, conductivity.shear(), conductivity.shear() },
            // Node x4y1
            F1Contact{ x3y1, 2, conductivity.shear(), conductivity.shear() },
        };
        CHECK_THAT(fStructure.fContacts(), matchers::c2::RangeEquals(expected));
    }

    SECTION(".fContactsOf()") {
        auto const expected = std::span<F1Contact const>{&fStructure.fContacts()[2], 4};
        auto const result = fStructure.fContactsOf(x2y1);
        CHECK(result.data() == expected.data());
        CHECK(result.size() == expected.size());
    }

    SECTION(".fLinks()") {
        auto const expected = std::vector<F1Link>{
            {0,0},
            {0,1},
            {1,0},
            {0,2},
            {3,0},
            {0,1},
        };
        CHECK_THAT(fStructure.fLinks(), matchers::c2::RangeEquals(expected));
    }

    SECTION(".fNodes()") {
        auto makeNode = [&](bool isFoundation, F1Node::ContactIds contactIds) {
            auto result = F1Node{ blockWeight, isFoundation };
            result.contactIds = contactIds;
            return result;
        };
        auto const expected = std::vector<F1Node>{
            makeNode(false, {0,0}),
            makeNode(false, {0,1}),
            makeNode(true, {1,1}),
            makeNode(false, {2,4}),
            makeNode(false, {6,2}),
            makeNode(false, {8,1}),
            makeNode(false, {9,2}),
            makeNode(false, {11,1}),
        };
        CHECK_THAT(fStructure.fNodes(), matchers::c2::RangeEquals(expected));
    }

    SECTION(".normalizedG()") {
        CHECK(fStructure.normalizedG() == NormalizedVector3{ g });
    }

    SECTION(".structure()") {
        CHECK(&fStructure.structure() == &structure);
    }
}
