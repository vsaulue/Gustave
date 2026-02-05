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

#pragma once

#include <gustave/units/stdStrict/lib/Exponent.hpp>
#include <gustave/units/stdStrict/lib/UnitIdentifier.hpp>
#include <gustave/units/stdStrict/lib/Unit.hpp>

namespace gustave::units::stdStrict {
    struct UnitSystem {
    private:
        struct Kilogram : lib::BasicUnitIdentifier<"kg"> {};
        static constexpr auto kg = lib::makeUnitIdentifier<Kilogram>();

        struct Metre : lib::BasicUnitIdentifier<"m"> {};
        static constexpr auto m = lib::makeUnitIdentifier<Metre>();

        struct Second : lib::BasicUnitIdentifier<"s"> {};
        static constexpr auto s = lib::makeUnitIdentifier<Second>();
    public:
        // dimension one
        static constexpr auto one = lib::one;
        // base units
        static constexpr auto mass = lib::Unit{ kg };
        static constexpr auto length = lib::Unit{ m };
        static constexpr auto time = lib::Unit{ s };
        // derived natural units
        static constexpr auto acceleration = lib::Unit{ m / s / s };
        static constexpr auto area = lib::Unit{ m * m };
        static constexpr auto density = lib::Unit{ kg / m / m / m };
        static constexpr auto force = lib::Unit<"N", kg * m / s / s>{};
        static constexpr auto pressure = lib::Unit<"Pa", kg / m / s / s>{};
        static constexpr auto volume = lib::Unit{ m * m * m };
        // custom
        static constexpr auto potential = lib::Unit<"lP", m>{};
        static constexpr auto resistance = lib::Unit<"lR", length.unitId() / force.unitId()>{};
        static constexpr auto conductivity = lib::Unit<"lC", force.unitId() / length.unitId()>{};
    };
}
