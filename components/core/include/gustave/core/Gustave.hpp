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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/worlds/SyncWorld.hpp>

namespace gustave::core {
    template<cfg::cLibConfig auto libCfg_>
    struct Gustave {
    public:
        [[nodiscard]]
        static constexpr auto libConfig() {
            return libCfg_;
        }

        [[nodiscard]]
        static constexpr auto units() {
            return cfg::units(libCfg_);
        }

        using RealRep = typename decltype(libCfg_)::RealRep;

        using NormalizedVector3 = cfg::NormalizedVector3<libCfg_>;

        template<cfg::cUnitOf<libCfg_> auto unit>
        using Real = cfg::Real<libCfg_, unit>;

        template<cfg::cUnitOf<libCfg_> auto unit>
        using Vector3 = cfg::Vector3<libCfg_, unit>;

        struct Worlds {
            using SyncWorld = gustave::core::worlds::SyncWorld<libCfg_>;
        };

        template<gustave::cfg::cUnitOf<libCfg_> Unit>
        [[nodiscard]]
        static constexpr auto vector3(RealRep x, RealRep y, RealRep z, Unit unit) {
            return Vector3<Unit{}>{ x, y, z, unit };
        }
    };
}
