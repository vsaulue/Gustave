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

#pragma once

#include <gustave/units/stdUnitless/lib/Unit.hpp>

namespace gustave::units::stdUnitless {
    struct UnitSystem {
    public:
        // dimension one
        static constexpr auto one = lib::Unit{};
        // base units
        static constexpr auto mass = lib::Unit{};
        static constexpr auto length = lib::Unit{};
        static constexpr auto time = lib::Unit{};
        // derived natural units
        static constexpr auto acceleration = lib::Unit{};
        static constexpr auto area = lib::Unit{};
        static constexpr auto density = lib::Unit{};
        static constexpr auto force = lib::Unit{};
        static constexpr auto pressure = lib::Unit{};
        static constexpr auto volume = lib::Unit{};
        // custom
        static constexpr auto potential = lib::Unit{};
        static constexpr auto resistance = lib::Unit{};
        static constexpr auto conductivity = lib::Unit{};
    };
}
