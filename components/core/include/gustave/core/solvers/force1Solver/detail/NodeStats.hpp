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
#include <gustave/core/solvers/force1Solver/detail/f1Structure/F1Node.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<cfg::cLibConfig auto libCfg>
    class NodeStats {
    private:
        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;
    public:
        using F1Node = f1Structure::F1Node<libCfg>;

        [[nodiscard]]
        explicit NodeStats(F1Node const& fNode, Real<u.force> force, Real<u.conductivity> conductivity)
            : fNode_{ &fNode }
            , force_{ force }
            , conductivity_{ conductivity }
        {}

        [[nodiscard]]
        Real<u.one> relativeError() const {
            return rt.abs(force_ / fNode_->weight);
        }

        [[nodiscard]]
        F1Node const& fNode() const {
            return *fNode_;
        }

        [[nodiscard]]
        Real<u.force> force() const {
            return force_;
        }

        [[nodiscard]]
        Real<u.conductivity> conductivity() const {
            return conductivity_;
        }
    private:
        F1Node const* fNode_;
        Real<u.force> force_;
        Real<u.conductivity> conductivity_;
    };
}
