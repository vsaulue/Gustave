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

#include <catch2/catch_test_macros.hpp>

#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/math3d/Vector3.hpp>
#include <gustave/units/RealTraits.hpp>

namespace G = Gustave;
namespace Cfg = Gustave::Cfg;
namespace M = Gustave::Testing::Matchers;

using Rep = double;

using Rt = Gustave::Units::RealTraits<Rep>;
inline constexpr Rt rt;
inline constexpr auto u = rt.units();

template<Cfg::cUnitOf<rt> auto unit>
using Real = Rt::Type<unit>;

template<Cfg::cUnitOf<rt> auto unit>
using Vector3 = Gustave::Math3d::Vector3<rt, unit>;

inline constexpr Cfg::cUnitOf<rt> auto l2 = u.length * u.length;
