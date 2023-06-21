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

#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/testing/Matchers.hpp>
#include <gustave/vanilla/Gustave.hpp>

// Aliases

namespace G = ::Gustave::Vanilla;
namespace C = ::Gustave::Cfg;
namespace M = ::Gustave::Testing::Matchers;

inline constexpr auto u = C::units(G::libConfig);
inline constexpr float epsilon{ 0.0001f };

// Constants

struct Normals {
    static inline G::NormalizedVector3 const x{1.f, 0.f, 0.f};
    static inline G::NormalizedVector3 const y{0.f, 1.f, 0.f};
    static inline G::NormalizedVector3 const z{0.f, 0.f, 1.f};
};

inline G::Vector3<u.acceleration> const g{ 0.f, -10.f, 0.f, u.acceleration };
inline G::Material const concrete_20m{
    20'000'000.f * u.pressure, // compressive
    14'000'000.f * u.pressure, // shear
    2'000'000.f * u.pressure, // tensile
};

// Utils

template<C::cUnitOf<G::libConfig> Unit>
[[nodiscard]]
static inline G::Vector3<Unit{}> vector3(std::floating_point auto x, std::floating_point auto y, std::floating_point auto z, Unit unit) {
    return {x, y, z, unit};
}
