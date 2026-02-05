/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <sstream>
#include <stdexcept>
#include <string>

#include <catch2/matchers/catch_matchers_templated.hpp>

#include <gustave/cfg/cReal.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::testing::matchers {
    template<cfg::cReal TargetReal>
    class RealWithinRelMatcher : public Catch::Matchers::MatcherGenericBase {
    public:
        using Rep = TargetReal::Rep;

        [[nodiscard]]
        constexpr RealWithinRelMatcher(TargetReal target, Rep epsilon)
            : target_(target)
            , epsilon_(epsilon)
        {
            if (epsilon < 0.0f || epsilon >= 1.0f) {
                throw std::domain_error("epsilon must be in [0;1[.");
            }
        }

        template<cfg::cReal TestedReal>
        [[nodiscard]]
        constexpr bool match(TestedReal tested) const {
            static_assert(TestedReal::unit().isAssignableFrom(TargetReal::unit()), "Invalid addition: incompatible units.");
            using RealType = decltype(meta::value(target_ + tested));
            auto const testedValue = RealType{ tested }.value();
            auto const targetValue = RealType{ target_ }.value();
            auto const margin = epsilon_ * std::max(std::fabs(testedValue), std::fabs(targetValue));
            return std::fabs(targetValue - testedValue) <= margin;
        }

        [[nodiscard]]
        std::string describe() const override {
            std::stringstream result;
            result << "and '" << target_ << "' are within " << epsilon_ * 100.0f << "% of each other.";
            return result.str();
        }
    private:
        TargetReal target_;
        Rep epsilon_;
    };

    template<cfg::cReal Target>
    RealWithinRelMatcher(Target, typename Target::Rep) -> RealWithinRelMatcher<Target>;

    [[nodiscard]]
    auto WithinRel(cfg::cReal auto target, std::floating_point auto epsilon) {
        using Rep = decltype(target.value() + epsilon);
        return RealWithinRelMatcher{ Rep{target.value()} * target.unit(), Rep{epsilon} };
    }
}
