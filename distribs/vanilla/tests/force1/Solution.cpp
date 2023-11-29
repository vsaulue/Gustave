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

#include <cstdint>
#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <gustave/vanilla/Gustave.hpp>
#include <gustave/solvers/force1/Config.hpp>

#include <TestConfig.hpp>

using Config = Gustave::Solvers::Force1::Config<G::libConfig>;
using Solution = Gustave::Solvers::Force1::Solution<G::libConfig>;

using Node = Solution::Structure::Node;

TEST_CASE("Force1::Solution") {
    auto structure = std::make_shared<G::SolverStructure>();
    for (unsigned i = 1; i <= 7; ++i) {
        structure->addNode(Node{ (i * 1'000.f) * u.mass, i == 1 });
    }
    structure->addLink({ 1, 0,  Normals::x, 1.f * u.area, 1.f * u.length, concrete_20m });
    structure->addLink({ 2, 0, -Normals::x, 1.f * u.area, 1.f * u.length, concrete_20m });
    structure->addLink({ 3, 0,  Normals::y, 2.f * u.area, 1.f * u.length, concrete_20m });
    structure->addLink({ 4, 0, -Normals::y, 2.f * u.area, 1.f * u.length, concrete_20m });
    structure->addLink({ 5, 0,  Normals::z, 1.f * u.area, 2.f * u.length, concrete_20m });
    structure->addLink({ 6, 0, -Normals::z, 1.f * u.area, 2.f * u.length, concrete_20m });

    constexpr float precision = 0.001f;
    auto config = std::make_shared<Config const>(g, 1000, precision);
    auto basis = std::make_shared<Solution::Basis>(structure, config);
    Solution solution{ basis };
    auto potentials = basis->spanPotentials();
    for (unsigned i = 0; i < potentials.size(); ++i) {
        potentials[i] = float(i*i) / 1000.f * u.potential;
    }

    SECTION("::force(NodeIndex, Nodeindex)") {
        auto runTest = [&solution](G::NodeIndex const to, G::NodeIndex const from, G::Vector3<u.force> const& expected) {
            CHECK_THAT(solution.forceVector(to, from), M::WithinRel(expected, epsilon));
            CHECK_THAT(solution.forceVector(from, to), M::WithinRel(-expected, epsilon));
        };

        SECTION("// 0-1") {
            runTest(0, 1, { 0.f, -14'000.f, 0.f, u.force });
        }

        SECTION("// 0-2") {
            runTest(0, 2, { 0.f, -56'000.f, 0.f, u.force });
        }

        SECTION("// 0-3") {
            runTest(0, 3, { 0.f, -36'000.f, 0.f, u.force });
        }

        SECTION("// 0-4") {
            runTest(0, 4, { 0.f, -640'000.f, 0.f, u.force });
        }

        SECTION("// 0-5") {
            runTest(0, 5, { 0.f, -175'000.f, 0.f, u.force });
        }

        SECTION("// 0-6") {
            runTest(0, 6, { 0.f, -252'000.f, 0.f, u.force });
        }

        SECTION("// 1-3") {
            runTest(1, 3, G::Vector3<u.force>::zero());
        }
    }

    SECTION("::relativeErrorOf(NodeIndex)") {
        SECTION("// 0") {
            CHECK_THAT(solution.relativeErrorOf(0), M::WithinRel(118.3f * u.one, epsilon));
        }
        SECTION("// 1") {
            CHECK_THAT(solution.relativeErrorOf(1), M::WithinRel(0.3f * u.one, epsilon));
        }
        SECTION("// 2") {
            CHECK_THAT(solution.relativeErrorOf(2), M::WithinRel((26.f / 30.f) * u.one, epsilon));
        }
        SECTION("// 3") {
            CHECK_THAT(solution.relativeErrorOf(3), M::WithinRel(0.1f * u.one, epsilon));
        }
        SECTION("// 4") {
            CHECK_THAT(solution.relativeErrorOf(4), M::WithinRel(11.8f * u.one, epsilon));
        }
        SECTION("// 5") {
            CHECK_THAT(solution.relativeErrorOf(5), M::WithinRel((115.f / 60.f) * u.one, epsilon));
        }
        SECTION("// 6") {
            CHECK_THAT(solution.relativeErrorOf(6), M::WithinRel((182.f / 70.f) * u.one, epsilon));
        }
    }

    SECTION("::maxRelativeError()") {
        CHECK_THAT(solution.maxRelativeError(), M::WithinRel(11.8f * u.one, epsilon));
    }

    SECTION("::sumRelativeError()") {
        CHECK_THAT(solution.sumRelativeError(), M::WithinRel(17.583333f * u.one, epsilon));
    }
}
