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
#include <cstdint>
#include <numeric>
#include <type_traits>

#include <gustave/utils/NumberHyperscript.hpp>
#include <gustave/utils/SizedString.hpp>

namespace gustave::units::lib {
    using ExpNum = std::int64_t;
    using ExpDen = std::uint64_t;

    namespace detail {
        [[nodiscard]]
        constexpr auto makeSigned(std::unsigned_integral auto const v) {
            return static_cast<std::make_signed_t<decltype(v)>>(v);
        }

        [[nodiscard]]
        constexpr std::pair<ExpNum, ExpDen> simplify(ExpNum num, ExpDen den) {
            if (num == 0) {
                return {0,1};
            } else {
                const auto gcd = makeSigned(std::gcd(num, den));
                return { num / gcd, den / gcd };
            }
        }

        [[nodiscard]]
        constexpr bool isSimplified(ExpNum num, ExpDen den) {
            const auto simplified = simplify(num, den);
            return (num == simplified.first) && (den == simplified.second);
        }

        struct ExponentText {
            static constexpr utils::SizedStringView fractionText = "\u2044";
        };
    }

    template<ExpNum num_, ExpDen den_ = 1>
    class Exponent {
    public:
        static_assert(den_ > 0, "Denominator cannot be 0.");
        static_assert(detail::isSimplified(num_,den_), "Only simplified fractions are allowed.");

        [[nodiscard]]
        constexpr Exponent() = default;

        [[nodiscard]]
        static constexpr ExpNum num() {
            return num_;
        }

        [[nodiscard]]
        static constexpr ExpDen den() {
            return den_;
        }

        [[nodiscard]]
        constexpr auto operator-() const {
            return Exponent<-num(), den()>();
        }

        [[nodiscard]]
        static constexpr bool isZero() {
            return num() == 0;
        }

        [[nodiscard]]
        static constexpr utils::cSizedString auto toString() {
            constexpr utils::NumberHyperscript numDec{num()};
            utils::SizedStringView<char, numDec.size()> numText{ numDec.begin() };
            if constexpr (den() == 1) {
                return utils::SizedString{ numText };
            } else {
                constexpr utils::NumberHyperscript denDec{ den() };
                utils::SizedStringView<char, denDec.size()> denText{ denDec.begin() };
                return utils::SizedString{ numText, detail::ExponentText::fractionText, denText };
            }
        }
    };

    template<typename T>
    concept cExponent = std::same_as<T,Exponent<T::num(),T::den()>>;

    namespace detail {
        template<ExpNum num, ExpDen den>
        [[nodiscard]]
        constexpr auto makeExponent() {
            constexpr auto simplified = simplify(num, den);
            return Exponent<simplified.first, simplified.second>();
        }
    }

    [[nodiscard]]
    constexpr auto operator+(cExponent auto lhs, cExponent auto rhs) {
        constexpr auto num = lhs.num() * detail::makeSigned(rhs.den()) + rhs.num() * detail::makeSigned(lhs.den());
        constexpr auto den = lhs.den() * rhs.den();
        return detail::makeExponent<num, den>();
    }

    [[nodiscard]]
    constexpr auto operator-(cExponent auto lhs, cExponent auto rhs) {
        return lhs + (-rhs);
    }

    [[nodiscard]]
    constexpr auto operator*(cExponent auto lhs, cExponent auto rhs) {
        constexpr auto num = lhs.num() * rhs.num();
        constexpr auto den = lhs.den() * rhs.den();
        return detail::makeExponent<num, den>();
    }

    [[nodiscard]]
    constexpr bool operator==(cExponent auto lhs, cExponent auto rhs) {
        return std::is_same_v<decltype(lhs), decltype(rhs)>;
    }
}
