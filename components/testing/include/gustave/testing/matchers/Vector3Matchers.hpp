/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <cmath>
#include <concepts>
#include <stdexcept>

#include <catch2/matchers/catch_matchers_templated.hpp>

#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/cVector3.hpp>

namespace gustave::testing::matchers {
    template<cfg::cVector3 TargetVector>
    class Vector3WithinRelMatcher : public Catch::Matchers::MatcherGenericBase {
    private:
        static constexpr auto rt = TargetVector::realTraits();
        static constexpr auto u = rt.units();
    public:
        using Coord = typename TargetVector::Coord;
        using RealRep = typename TargetVector::RealRep;

        template<cfg::cUnitOf<rt> auto unit>
        using Real = typename decltype(rt)::template Type<unit, RealRep>;

        [[nodiscard]]
        constexpr Vector3WithinRelMatcher(TargetVector const& target, Real<u.one> epsilon)
            : target_{ target }
            , epsilon_{ epsilon }
        {
            if (epsilon < 0.0f || epsilon >= 1.0f) {
                throw std::domain_error("epsilon must be in [0;1[.");
            }
        }

        template<cfg::cVector3 TestedVector>
        [[nodiscard]]
        constexpr bool match(TestedVector tested) const {
            static_assert(TestedVector::realTraits() == TargetVector::realTraits(), "Invalid comparison: incompatible traits.");
            static_assert(TestedVector::unit().isAssignableFrom(TargetVector::unit()), "Invalid comparison: incompatible units.");
            auto const testedNorm = tested.norm();
            auto const targetNorm = target_.norm();
            auto const deltaNorm = (target_ - tested).norm();
            auto const margin = epsilon_ * rt.max(rt.abs(testedNorm), rt.abs(targetNorm));
            return deltaNorm <= margin;
        }

        [[nodiscard]]
        std::string describe() const override {
            std::stringstream result;
            result << "and '" << target_ << "' are within " << epsilon_ * 100.0f << "% of each other.";
            return result.str();
        }
    private:
        TargetVector target_;
        Real<u.one> epsilon_;
    };

    template<cfg::cVector3 TargetVector>
    Vector3WithinRelMatcher(TargetVector, typename TargetVector::RealRep) -> Vector3WithinRelMatcher<TargetVector>;

    [[nodiscard]]
    auto WithinRel(cfg::cVector3 auto target, std::floating_point auto epsilon) {
        using Rep = decltype(meta::value(target.x().value() + epsilon));
        return Vector3WithinRelMatcher{ target, Rep{epsilon} };
    }
}
