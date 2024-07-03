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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/NodePoint.hpp>

namespace gustave::core::solvers::force1Solver::detail {
    template<typename T, auto libCfg>
    concept cNodeEvaluatorOf = requires (T const& cv) {
        requires cfg::cLibConfig<decltype(libCfg)>;
        { cv.pointAt(0.f * cfg::units(libCfg).potential) } -> std::same_as<NodePoint<libCfg>>;
        { cv.weight() } -> std::convertible_to<cfg::Real<libCfg,cfg::units(libCfg).force>>;
    };

    template<cfg::cLibConfig auto libCfg>
    class NodeBalancer {
    private:
        static constexpr auto u = cfg::units(libCfg);
        static constexpr auto rt = libCfg.realTraits;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        struct Result {
            Real<u.potential> offset;
            Real<u.force> initialForce;
        };

        [[nodiscard]]
        explicit NodeBalancer(Real<u.one> maxErrorFactor)
            : maxErrorFactor_{ maxErrorFactor }
        {
            assert(maxErrorFactor_ > 0.f);
        }

        [[nodiscard]]
        Result findBalanceOffset(cNodeEvaluatorOf<libCfg> auto const& evaluator, Real<u.potential> startPotential) const {
            Real<u.force> const maxForceError = maxErrorFactor_ * evaluator.weight();
            assert(maxForceError > 0.f * u.force);
            auto curPoint = evaluator.pointAt(startPotential);
            Real<u.force> const initialForce = curPoint.force();
            auto nextPoint = evaluator.pointAt(curPoint.nextOffset());
            if (rt.abs(nextPoint.force()) <= maxForceError) {
                return { nextPoint.offset(), initialForce };
            }
            bool const startSignBit = rt.signBit(curPoint.force());
            while (rt.signBit(nextPoint.force()) == startSignBit) {
                curPoint = nextPoint;
                nextPoint = evaluator.pointAt(curPoint.nextOffset());
                assert(curPoint.offset() != nextPoint.offset()); // float resolution too low ?
                if (rt.abs(nextPoint.force()) <= maxForceError) {
                    return { nextPoint.offset(), initialForce };
                }
            }
            do {
                Real<u.resistance> const invDerivative = (nextPoint.offset() - curPoint.offset()) / (nextPoint.force() - curPoint.force());
                auto const midPoint = evaluator.pointAt(curPoint.offset() - invDerivative * curPoint.force());
                assert(midPoint.offset() != curPoint.offset());// float resolution too low ?
                assert(midPoint.offset() != nextPoint.offset()); // float resolution too low ?
                if (rt.abs(midPoint.force()) <= maxForceError) {
                    return { midPoint.offset(), initialForce };
                }
                if (rt.signBit(midPoint.offset()) == startSignBit) {
                    curPoint = midPoint;
                } else {
                    nextPoint = midPoint;
                }
            } while (true);
        }
    private:
        Real<u.one> maxErrorFactor_;
    };
}
