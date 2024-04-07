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
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>

namespace gustave::solvers::force1Solver {
    template<cfg::cLibConfig auto libCfg>
    class Config {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = typename cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = typename cfg::Vector3<libCfg, unit>;
    public:
        using IterationIndex = std::uint64_t;

        [[nodiscard]]
        explicit Config(Vector3<u.acceleration> const& g, IterationIndex maxIterations, Real<u.one> targetMaxError)
            : g_{ g }
            , maxIterations_{ maxIterations }
            , targetMaxError_{ targetMaxError }
        {
            assert(targetMaxError > 0.f);
        }

        [[nodiscard]]
        Vector3<u.acceleration> const& g() const {
            return g_;
        }

        [[nodiscard]]
        IterationIndex maxIterations() const {
            return maxIterations_;
        }

        [[nodiscard]]
        Real<u.one> targetMaxError() const {
            return targetMaxError_;
        }
    private:
        Vector3<u.acceleration> g_;
        IterationIndex maxIterations_;
        Real<u.one> targetMaxError_;
    };
}
