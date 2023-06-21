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

#include <concepts>
#include <ostream>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/cfg/cReal.hpp>
#include <gustave/cfg/cUnit.hpp>
#include <gustave/meta/Meta.hpp>

namespace Gustave::Cfg {
    namespace detail {
        template<typename To, auto from>
        concept cUnitAssignableFrom = requires {
            requires cUnit<To>;
            requires cUnit<decltype(from)>;
            requires To::isAssignableFrom(from);
        };

        template<typename T>
        concept cUnitSystem = requires(T u) {
            { Meta::value(u.acceleration) } -> cUnit;
            { Meta::value(u.area) } -> cUnit;
            { Meta::value(u.conductivity) } -> cUnit;
            { Meta::value(u.force) } -> cUnit;
            { Meta::value(u.length) } -> cUnit;
            { Meta::value(u.mass) } -> cUnit;
            { Meta::value(u.one) } -> cUnit;
            { Meta::value(u.potential) } -> cUnit;
            { Meta::value(u.pressure) } -> cUnit;
            { Meta::value(u.resistance) } -> cUnit;
            { Meta::value(u.time) } -> cUnit;

            // one is identity
            { u.one * u.area } -> cUnitAssignableFrom<T::area>;
            { u.area * u.one } -> cUnitAssignableFrom<T::area>;
            { u.area / u.one } -> cUnitAssignableFrom<T::area>;

            // relation between base & derived units.
            { u.length / u.time / u.time } -> cUnitAssignableFrom<T::acceleration>;
            { u.mass * u.acceleration } -> cUnitAssignableFrom<T::force>;
            { u.length * u.length } -> cUnitAssignableFrom<T::area>;
            { u.force / u.area } -> cUnitAssignableFrom<T::pressure>;
            { Meta::value(u.potential) } -> cUnitAssignableFrom<T::length>;
            { u.potential / u.resistance } -> cUnitAssignableFrom<T::force>;
            { u.potential * u.conductivity } -> cUnitAssignableFrom<T::force>;
        };

        template<typename RealTraits, auto unit>
        using Real = typename RealTraits::template Type<unit>;
    }

    template<typename T>
    concept cRealTraits = requires(T traits) {
        { traits == traits } -> std::convertible_to<bool>;

        // ::units()
        { traits.units() } -> detail::cUnitSystem;

        // ::Type<unit> existence
        requires cReal<detail::Real<T, T::units().mass>>;

        // ::Type<unit> arithmetics rules
        requires requires (
            detail::Real<T, T::units().acceleration> acceleration,
            detail::Real<T, T::units().area> area,
            detail::Real<T, T::units().force> force,
            detail::Real<T, T::units().length> length,
            detail::Real<T, T::units().mass> mass,
            detail::Real<T, T::units().one> one,
            detail::Real<T, T::units().potential> potential,
            detail::Real<T, T::units().pressure> pressure,
            detail::Real<T, T::units().resistance> resistance,
            detail::Real<T, T::units().time> time
        ) {
            { Meta::value(acceleration) } -> cReal;
            { Meta::value(area) } -> cReal;
            { Meta::value(force) } -> cReal;
            { Meta::value(length) } -> cReal;
            { Meta::value(mass) } -> cReal;
            { Meta::value(one) } -> cReal;
            { Meta::value(potential) } -> cReal;
            { Meta::value(pressure) } -> cReal;
            { Meta::value(resistance) } -> cReal;
            { Meta::value(time) } -> cReal;

            acceleration = length / time / time;
            area = length * length;
            force = mass * acceleration;
            potential = one;
            pressure = force / area;
            resistance = potential / force;

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
        };

        // ::sqrt(Real) -> Real
        requires requires (detail::Real<T, T::units().length> length) {
            { T::sqrt(length * length) } -> std::convertible_to<decltype(length)>;
        };
    };
}
