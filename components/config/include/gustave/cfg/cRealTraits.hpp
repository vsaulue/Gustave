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

#include <concepts>
#include <ostream>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/cfg/cReal.hpp>
#include <gustave/cfg/cUnit.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::cfg {
    namespace detail {
        template<typename To, auto from>
        concept cUnitAssignableFrom = requires {
            requires cUnit<To>;
            requires cUnit<decltype(from)>;
            requires To::isAssignableFrom(from);
        };

        template<typename T>
        concept cUnitSystem = requires(T u) {
            { meta::value(u.acceleration) } -> cUnit;
            { meta::value(u.area) } -> cUnit;
            { meta::value(u.conductivity) } -> cUnit;
            { meta::value(u.density) } -> cUnit;
            { meta::value(u.force) } -> cUnit;
            { meta::value(u.length) } -> cUnit;
            { meta::value(u.mass) } -> cUnit;
            { meta::value(u.one) } -> cUnit;
            { meta::value(u.potential) } -> cUnit;
            { meta::value(u.pressure) } -> cUnit;
            { meta::value(u.resistance) } -> cUnit;
            { meta::value(u.time) } -> cUnit;
            { meta::value(u.volume) } -> cUnit;

            // one is identity
            { u.one * u.area } -> cUnitAssignableFrom<T::area>;
            { u.area * u.one } -> cUnitAssignableFrom<T::area>;
            { u.area / u.one } -> cUnitAssignableFrom<T::area>;

            // relation between base & derived units.
            { u.length / u.time / u.time } -> cUnitAssignableFrom<T::acceleration>;
            { u.mass * u.acceleration } -> cUnitAssignableFrom<T::force>;
            { u.length * u.length } -> cUnitAssignableFrom<T::area>;
            { u.force / u.area } -> cUnitAssignableFrom<T::pressure>;
            { meta::value(u.potential) } -> cUnitAssignableFrom<T::length>;
            { u.potential / u.resistance } -> cUnitAssignableFrom<T::force>;
            { u.potential * u.conductivity } -> cUnitAssignableFrom<T::force>;
            { u.length * u.length * u.length } -> cUnitAssignableFrom<T::volume>;
            { u.density * u.volume } -> cUnitAssignableFrom<T::mass>;
        };

        template<typename RealTraits, auto unit>
        using Float = RealTraits::template Type<unit,float>;
    }

    template<typename T>
    concept cRealTraits = requires(T traits) {
        { traits == traits } -> std::convertible_to<bool>;

        // ::units()
        { traits.units() } -> detail::cUnitSystem;

        // ::Type<unit> existence
        requires cReal<detail::Float<T, T::units().mass>>;

        requires requires (
            detail::Float<T, T::units().acceleration> acceleration,
            detail::Float<T, T::units().area> area,
            detail::Float<T, T::units().density> density,
            detail::Float<T, T::units().force> force,
            detail::Float<T, T::units().length> length,
            detail::Float<T, T::units().mass> mass,
            detail::Float<T, T::units().one> one,
            detail::Float<T, T::units().potential> potential,
            detail::Float<T, T::units().pressure> pressure,
            detail::Float<T, T::units().resistance> resistance,
            detail::Float<T, T::units().time> time,
            detail::Float<T, T::units().volume> volume
        ) {
            { meta::value(acceleration) } -> cReal;
            { meta::value(area) } -> cReal;
            { meta::value(density) } -> cReal;
            { meta::value(force) } -> cReal;
            { meta::value(length) } -> cReal;
            { meta::value(mass) } -> cReal;
            { meta::value(one) } -> cReal;
            { meta::value(potential) } -> cReal;
            { meta::value(pressure) } -> cReal;
            { meta::value(resistance) } -> cReal;
            { meta::value(time) } -> cReal;
            { meta::value(volume) } -> cReal;

            // Real arithmetics rules
            acceleration = length / time / time;
            area = length * length;
            density = mass / length / length / length;
            force = mass * acceleration;
            potential = one;
            pressure = force / area;
            resistance = potential / force;
            volume = length * length * length;

            area = area * one;
            area = one * area;
            area = area / one;

            area *= one;
            area *= 1.0f;
            area /= one;
            area /= 1.0f;

            { one + 1.0f } -> cReal;
            { 1.0f + one } -> cReal;
            { one - 1.0f } -> cReal;
            { 1.0f - one } -> cReal;

            // Realtraits functions
            length = T::sqrt(length * length);
            area = T::max(length * length, area);
            area = T::min(area, length * length);
            { T::signBit(area) } -> std::convertible_to<bool>;
        };
    };
}
