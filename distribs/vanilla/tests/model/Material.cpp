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

#include <catch2/catch_test_macros.hpp>

#include <gustave/model/Material.hpp>

#include <TestConfig.hpp>

using Material = Gustave::Model::Material<G::libConfig>;

TEST_CASE("Model::Material") {
    SECTION("::minResistance(Material const&, Material const&)") {
        Material const m1{
            4.f * u.pressure, // compressive
            1.f * u.pressure, // shear
            7.f * u.pressure, // tensile
        };
        Material const m2{
            3.f * u.pressure,
            5.f * u.pressure,
            6.f * u.pressure,
        };
        Material const expected{
            3.f * u.pressure,
            1.f * u.pressure,
            6.f * u.pressure,
        };
        CHECK(Material::minResistance(m1, m2) == expected);
        CHECK(Material::minResistance(m2, m1) == expected);
    }
}
