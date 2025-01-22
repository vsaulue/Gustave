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
#include <type_traits>

#include <gustave/cfg/cPrintable.hpp>
#include <gustave/cfg/cRealOf.hpp>
#include <gustave/cfg/cRealRep.hpp>
#include <gustave/cfg/cRealTraits.hpp>
#include <gustave/cfg/cUnit.hpp>

namespace gustave::cfg {
    template<typename T>
    concept cVector3 = requires (T v, T const cv) {
        requires cPrintable<T>;
        { T::unit() } -> cUnit;
        { T::realTraits() } -> cRealTraits;

        requires cRealOf<typename T::Coord, T::realTraits()>;
        requires std::is_same_v<typename T::RealRep, typename T::Coord::Rep>;

        requires requires (typename T::Coord c) { T{ c,c,c }; };
        requires requires (typename T::RealRep rep) { T{ rep, rep, rep, T::unit() }; };
        { T::zero() } -> std::same_as<T>;

        v += cv;
        { cv + cv } -> std::same_as<T>;

        v -= cv;
        { cv - cv } -> std::same_as<T>;

        v *= 2.0f;
        v /= 2.0f;

        { v.x() } -> std::same_as<typename T::Coord&>;
        { cv.x() } -> std::convertible_to<typename T::Coord>;

        { v.y() } -> std::same_as<typename T::Coord&>;
        { cv.y() } -> std::convertible_to<typename T::Coord>;

        { v.z() } -> std::same_as<typename T::Coord&>;
        { cv.z() } -> std::convertible_to<typename T::Coord>;

        { cv.norm() } -> std::same_as<typename T::Coord>;
        { cv.isCompatible(T::realTraits().units().one) } -> std::same_as<bool>;

        { 2.0f * cv } -> std::convertible_to<T>;
        { cv / 2.0f } -> std::convertible_to<T>;
    };
}
