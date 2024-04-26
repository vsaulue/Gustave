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

TEST_CASE("UnitSystem") {
    SECTION("// length * length <-> area") {
        CHECK((10.f * u.length) * (4.f * u.length) == 40.f * u.area);
        CHECK((40.f * u.area) / (10.f * u.length) == 4.f * u.length);
    }

    SECTION("// length <-> acceleration * time * time") {
        CHECK(30.f * u.length == (2.f * u.acceleration) * (3.f * u.time) * (5.f * u.time));
        CHECK(30.f * u.length / (3.f * u.time) / (5.f * u.time)  == (2.f * u.acceleration));
    }

    SECTION("// mass <-> density * volume") {
        CHECK(20.f * u.mass == (4.f * u.density) * (5.f * u.volume));
        CHECK((20.f * u.mass) / (5.f * u.volume) == 4.f * u.density);
    }

    SECTION("// mass * acceleration <-> force") {
        CHECK((3.f * u.mass) * (10.f * u.acceleration) == 30.f * u.force);
        CHECK((30.f * u.force) / (10.f * u.acceleration) == 3.f * u.mass);
    }

    SECTION("// pressure * area <-> force") {
        CHECK((4.f * u.pressure) * (3.f * u.area) == 12.f * u.force);
        CHECK((12.f * u.force) / (3.f * u.area) == 4.f * u.pressure);
    }

    SECTION("// length * length * length <-> volume") {
        CHECK((2.f * u.length) * (3.f * u.length) * (5.f * u.length) == 30.f * u.volume);
        CHECK((30.f * u.volume) / (3.f * u.length) / (2.f * u.length) == 5.f * u.length);
    }

    SECTION("// length <-> potential") {
        CHECK(2.f * u.length == 2.f * u.potential);
    }

    SECTION("// conductivity * potential <-> force") {
        CHECK((2.f * u.conductivity) * (3.f * u.potential) == 6.f * u.force);
        CHECK((6.f * u.force) / (3.f * u.conductivity) == 2.f * u.potential);
    }

    SECTION("// conductivity * resistance <-> one") {
        CHECK((2.f * u.conductivity) * (3.f * u.resistance) == 6.f * u.one);
        CHECK((6.f * u.one) / (3.f * u.conductivity) == 2.f * u.resistance);
    }
}
