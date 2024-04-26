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

#include <gustave/distribs/std/strictUnit/LibConfig.hpp>
#include <gustave/core/worlds/SyncWorld.hpp>

namespace gustave::distribs::std::strictUnit {
    struct Gustave {
    public:
        static constexpr LibConfig libConfig{};

        using Rep = gustave::cfg::Real<libConfig, gustave::cfg::units(libConfig).one>::Rep;

        using NormalizedVector3 = gustave::cfg::NormalizedVector3<libConfig>;

        template<gustave::cfg::cUnitOf<libConfig> auto unit>
        using Real = gustave::cfg::Real<libConfig, unit>;

        template<gustave::cfg::cUnitOf<libConfig> auto unit>
        using Vector3 = gustave::cfg::Vector3<libConfig, unit>;

        [[nodiscard]]
        static constexpr auto units() {
            return gustave::cfg::units(libConfig);
        }

        struct Worlds {
            using SyncWorld = gustave::core::worlds::SyncWorld<libConfig>;
        };

        template<gustave::cfg::cUnitOf<libConfig> Unit>
        [[nodiscard]]
        static constexpr auto vector3(Rep x, Rep y, Rep z, Unit unit) {
            return Vector3<Unit{}>{ x, y, z, unit };
        }
    };
}
