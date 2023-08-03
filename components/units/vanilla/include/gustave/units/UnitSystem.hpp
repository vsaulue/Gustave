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

#pragma once

#include <gustave/units/lib/Exponent.hpp>
#include <gustave/units/lib/UnitIdentifier.hpp>
#include <gustave/units/lib/Unit.hpp>

namespace Gustave::Units {
    struct UnitSystem {
    private:
        struct Kilogram : Lib::BasicUnitIdentifier<"kg"> {};
        static constexpr auto kg = Lib::makeUnitIdentifier<Kilogram>();

        struct Metre : Lib::BasicUnitIdentifier<"m"> {};
        static constexpr auto m = Lib::makeUnitIdentifier<Metre>();

        struct Second : Lib::BasicUnitIdentifier<"s"> {};
        static constexpr auto s = Lib::makeUnitIdentifier<Second>();
    public:
        // dimension one
        static constexpr auto one = Lib::one;
        // base units
        static constexpr auto mass = Lib::Unit{ kg };
        static constexpr auto length = Lib::Unit{ m };
        static constexpr auto time = Lib::Unit{ s };
        // derived natural units
        static constexpr auto acceleration = Lib::Unit{ m / s / s };
        static constexpr auto area = Lib::Unit{ m * m };
        static constexpr auto density = Lib::Unit{ kg / m / m / m };
        static constexpr auto force = Lib::Unit<"N", kg * m / s / s>{};
        static constexpr auto pressure = Lib::Unit<"Pa", kg / m / s / s>{};
        static constexpr auto volume = Lib::Unit{ m * m * m };
        // custom
        static constexpr auto potential = Lib::Unit<"lP", m>{};
        static constexpr auto resistance = Lib::Unit<"lR", length.unitId() / force.unitId()>{};
        static constexpr auto conductivity = Lib::Unit<"lC", force.unitId() / length.unitId()>{};
    };
}
