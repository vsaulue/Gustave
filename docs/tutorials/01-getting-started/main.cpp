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

#include <iostream>

#include <Tutorial.hpp>

// -8<- [start:include-unitless]
// One global header per distribution
#include <gustave/distribs/std/unitless/Gustave.hpp>
// -8<- [end:include-unitless]

#include <gustave/distribs/std/strictUnit/Gustave.hpp>

static void strictUnitDemo() {
    // Choosing a distribution, with double precision
    using G = gustave::distribs::std::strictUnit::Gustave<double>;

    // All units/dimensions used by Gustave (internally & API).
    static constexpr auto u = G::units();
    // `G::Real<u.length>` is a float representing a distance (in metre).
    // `G::Vector3<u.force>` is a force vector (in Newton).

    auto g = G::vector3(0, -9.8, 0, u.acceleration); // metre / second²
    auto mass = 1000.0 * u.mass; // kilogram
    G::Vector3<u.force> weight = mass * g; // Converted into Newton
    std::cout << "g = " << g << '\n';
    std::cout << "mass = " << mass << '\n';
    std::cout << "weight = mass * g = " << weight << '\n';

    // The next line won't compile: "mass + acceleration" is NOT a valid operation.
    // std::cout << "wrong_operation = mass + g.y() = " << mass + g.y() << '\n';
}

// -8<- [start:unitlessDemo]
static void unitlessDemo() {
    // Choosing a distribution, with double precision
    using G = gustave::distribs::std::unitless::Gustave<double>;

    auto g = G::vector3(0, -9.8, 0);
    auto mass = 1000.0;
    auto weight = mass * g;
    std::cout << "g = " << g << '\n';
    std::cout << "mass = " << mass << '\n';
    std::cout << "weight = mass * g = " << weight << '\n';
    std::cout << "wrong_operation = mass + g.y() = " << mass + g.y() << '\n';
}
// -8<- [end:unitlessDemo]

int main(int argc, char** argv) {
    auto tuto = Tutorial{ "Getting started", argc, argv };
    if (tuto.earlyExitCode()) {
        return *tuto.earlyExitCode();
    }

    tuto.section("strict-unit-demo", "Gustave 'StrictUnit' Demo");
    strictUnitDemo();

    tuto.section("unitless-demo", "Gustave 'Unitless' Demo");
    unitlessDemo();
    tuto.endSection();
}
