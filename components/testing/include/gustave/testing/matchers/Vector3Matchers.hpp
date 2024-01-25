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

#include <cmath>
#include <concepts>
#include <stdexcept>

#include <catch2/matchers/catch_matchers_templated.hpp>

#include <gustave/cfg/cVector3.hpp>

namespace Gustave::Testing::Matchers {
    template<Cfg::cVector3 TargetVector>
    class Vector3WithinRelMatcher : public Catch::Matchers::MatcherGenericBase {
    public:
        using Coord = typename TargetVector::Coord;
        using RealRep = typename TargetVector::RealRep;

        [[nodiscard]]
        constexpr Vector3WithinRelMatcher(TargetVector const& target, RealRep epsilon)
            : target_(target)
            , epsilon_(epsilon)
        {
            if (epsilon < 0.0f || epsilon >= 1.0f) {
                throw std::domain_error("epsilon must be in [0;1[.");
            }
        }

        template<Cfg::cVector3 TestedVector>
        [[nodiscard]]
        constexpr bool match(TestedVector tested) const {
            static_assert(TestedVector::realTraits() == TargetVector::realTraits(), "Invalid comparison: incompatible traits.");
            static_assert(TestedVector::unit().isAssignableFrom(TargetVector::unit()), "Invalid comparison: incompatible units.");
            const Coord testedNorm = tested.norm();
            const Coord targetNorm = target_.norm();
            const Coord deltaNorm = (target_ - tested).norm();
            const RealRep margin = epsilon_ * std::max(std::fabs(testedNorm.value()), std::fabs(targetNorm.value()));
            return deltaNorm.value() <= margin;
        }

        [[nodiscard]]
        std::string describe() const override {
            std::stringstream result;
            result << "and '" << target_ << "' are within " << epsilon_ * 100.0f << "% of each other.";
            return result.str();
        }
    private:
        TargetVector target_;
        RealRep epsilon_;
    };

    template<Cfg::cVector3 TargetVector>
    Vector3WithinRelMatcher(TargetVector, typename TargetVector::RealRep) -> Vector3WithinRelMatcher<TargetVector>;

    [[nodiscard]]
    auto WithinRel(Cfg::cVector3 auto target, std::floating_point auto epsilon) {
        using Rep = decltype(Meta::value(target.x().value() + epsilon));
        return Vector3WithinRelMatcher{ target, Rep{epsilon} };
    }
}
