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

#include <concepts>
#include <type_traits>

#include <gustave/cfg/cRealTraits.hpp>
#include <gustave/utils/SizedString.hpp>

namespace gustave::units::stdStrict::lib {
    template<utils::SizedString symbol_>
    struct BasicUnitIdentifier;

    template<typename T>
    concept cBasicUnitIdentifier = std::derived_from<T, BasicUnitIdentifier<T::symbol()>>;



    using ExpNum = std::int64_t;
    using ExpDen = std::uint64_t;

    template<ExpNum num_, ExpDen den_>
    class Exponent;

    template<typename T>
    concept cExponent = std::same_as<T, Exponent<T::num(), T::den()>>;



    template<cBasicUnitIdentifier auto basicUnit_, cExponent auto exponent_>
    struct UnitTerm;

    template<typename T>
    concept cUnitTerm = std::same_as<T, UnitTerm<T::basicUnit(), T::exponent()>>;



    template<UnitTerm... terms>
    class UnitIdentifier;

    namespace detail {
        template<typename T>
        struct IsUnitIdentifier : std::false_type {};

        template<cUnitTerm auto... terms>
        struct IsUnitIdentifier<UnitIdentifier<terms...>> : std::true_type {};
    };

    template<typename T>
    concept cUnitIdentifier = detail::IsUnitIdentifier<T>::value;



    template<utils::SizedString symbol_, cUnitIdentifier auto unitId_>
    class Unit;

    template<typename T>
    concept cUnit = std::same_as<T, Unit<T::symbol(), T::unitId()>>;



    template<cUnit auto unit_, cfg::cRealRep Rep_>
    class Real;

    template<typename T>
    concept cReal = std::same_as<T, Real<T::unit(), typename T::Rep>>;
}
