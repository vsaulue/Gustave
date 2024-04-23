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

#include <gustave/core/solvers/force1Solver/Config.hpp>

using Config = gustave::core::solvers::force1Solver::Config<libCfg>;

TEST_CASE("core::force1Solver::Config") {
    Config config{ g, 0.01f, 1000 };

    SECTION("// constructor & getters") {
        CHECK(config.g() == g);
        CHECK(config.targetMaxError() == 0.01f);
        CHECK(config.maxIterations() == 1000);
    }

    SECTION(".setMaxIterations()") {
        config.setMaxIterations(12345);
        CHECK(config.maxIterations() == 12345);
    }

    SECTION(".setG()") {
        Vector3<u.acceleration> newG = vector3(5.f, 0.f, 0.f, u.acceleration);
        config.setG(newG);
        CHECK(config.g() == newG);
    }

    SECTION(".setTargetMaxError()") {
        SECTION("// valid") {
            config.setTargetMaxError(0.125f);
            CHECK(config.targetMaxError() == 0.125f);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(config.setTargetMaxError(-0.125f), std::invalid_argument);
        }
    }
}
