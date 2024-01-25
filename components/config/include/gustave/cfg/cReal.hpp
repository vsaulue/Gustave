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

#pragma once

#include <concepts>

#include <gustave/cfg/cPrintable.hpp>
#include <gustave/cfg/cRealRep.hpp>
#include <gustave/cfg/cUnit.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Cfg {
    template<typename T>
    concept cReal = requires (T v, const T cv) {
        T{ Utils::NO_INIT };

        requires cPrintable<T>;
        requires cRealRep<typename T::Rep>;

        { cv.zero() } -> std::convertible_to<T>;
        { cv.unit() } -> cUnit;

        { cv.value() } -> std::same_as<typename T::Rep const&>;
        { v.value() } -> std::same_as<typename T::Rep&>;

        { -cv } -> std::convertible_to<T>;

        v += cv;
        { v + v } -> std::convertible_to<T>;

        v -= cv;
        { v - v } -> std::convertible_to<T>;

        cv * cv;
        { cv * 2.0f } -> std::convertible_to<T>;
        { 2.0f * cv } -> std::convertible_to<T>;

        cv / cv;
        { cv / 2.0f } -> std::convertible_to<T>;
        { 2.0f / cv } -> std::convertible_to<T>;

        { cv / cv - 1.0f } -> std::convertible_to<T>;
        { cv / cv + 1.0f } -> std::convertible_to<T>;
        { 1.0f - cv / cv } -> std::convertible_to<T>;
        { 1.0f + cv / cv } -> std::convertible_to<T>;
    };
}
