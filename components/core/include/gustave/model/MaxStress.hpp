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

#include <cassert>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/meta/Meta.hpp>

namespace Gustave::Model {
    template<Cfg::cLibConfig auto cfg>
    struct MaxStress {
    private:
        static constexpr auto u = Cfg::units(cfg);
        static constexpr auto rt = cfg.realTraits;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;
    public:
        [[nodiscard]]
        MaxStress(Real<u.pressure> maxCompression, Real<u.pressure> maxShear, Real<u.pressure> maxTensile)
            : maxCompressionStress_{maxCompression}
            , maxShearStress_{maxShear}
            , maxTensileStress_{maxTensile}
        {
            assert(maxCompression > 0.f * u.pressure);
            assert(maxShear > 0.f * u.pressure);
            assert(maxTensile > 0.f * u.pressure);
        }

        [[nodiscard]]
        static MaxStress minResistance(MaxStress const& m1, MaxStress const& m2) {
            return {
                rt.min(m1.maxCompressionStress_, m2.maxCompressionStress_),
                rt.min(m1.maxShearStress_, m2.maxShearStress_),
                rt.min(m1.maxTensileStress_, m2.maxTensileStress_)
            };
        }

        [[nodiscard]]
        Real<u.pressure> maxCompressionStress() const {
            return maxCompressionStress_;
        }

        [[nodiscard]]
        Real<u.pressure> maxShearStress() const {
            return maxShearStress_;
        }

        [[nodiscard]]
        Real<u.pressure> maxTensileStress() const {
            return maxTensileStress_;
        }

        [[nodiscard]]
        bool operator==(MaxStress const&) const = default;
    private:
        Real<u.pressure> maxCompressionStress_;
        Real<u.pressure> maxShearStress_;
        Real<u.pressure> maxTensileStress_;
    };
}
