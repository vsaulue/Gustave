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

#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/math3d/Vector3.hpp>
#include <TestConfig.hpp>

namespace G = ::Gustave;
inline constexpr RealTraits rt{};

inline constexpr auto u = rt.units();

template<G::Cfg::cUnitOf<rt> auto unit>
using Real = RealTraits::template Type<unit>;

template<G::Cfg::cUnitOf<rt> auto unit>
using Vector3 = G::Math3d::Vector3<rt, unit>;

template<G::Cfg::cUnitOf<rt> Unit, std::floating_point Float>
[[nodiscard]]
constexpr auto vector3(Float x, Float y, Float z, Unit unit) -> Vector3 < Unit{} > {
    return { x,y,z,unit };
}

inline constexpr float epsilon = 0.0001f;
