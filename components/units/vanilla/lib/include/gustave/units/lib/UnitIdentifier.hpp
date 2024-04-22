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

#include <type_traits>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/units/lib/concepts.hpp>
#include <gustave/units/lib/Exponent.hpp>
#include <gustave/units/lib/UnitTerm.hpp>
#include <gustave/utils/SizedString.hpp>

namespace gustave::units::lib {
    template<UnitTerm headTerm_, UnitTerm... tailTerms_>
    class UnitIdentifier<headTerm_, tailTerms_...> {
    public:
        using HeadTerm = decltype(headTerm_);
        using Char = typename HeadTerm::Char;
        using TailUnit = UnitIdentifier<tailTerms_...>;

        [[nodiscard]]
        constexpr UnitIdentifier() = default;

        [[nodiscard]]
        static constexpr HeadTerm headTerm() {
            return headTerm_;
        }

        [[nodiscard]]
        static constexpr TailUnit tailUnit() {
            return {};
        }

        [[nodiscard]]
        static constexpr bool isOne() {
            return false;
        }

        [[nodiscard]]
        static constexpr UnitIdentifier<-headTerm(), -tailTerms_...> inverse() {
            return {};
        }

        template<UnitTerm newHead>
        [[nodiscard]]
        static constexpr UnitIdentifier<newHead,headTerm(), tailTerms_...> addHead() {
            static_assert(newHead < headTerm(), "Terms in UnitIdentifier must be sorted in ascending order.");
            return {};
        }

        [[nodiscard]]
        static constexpr auto pow(cExponent auto exp);

        [[nodiscard]]
        static constexpr utils::cSizedString auto toString() {
            if constexpr (tailUnit().isOne()) {
                return headText();
            } else {
                return utils::SizedString{ headText().sizedView(), utils::SizedStringView{"."}, tailUnit().toString().sizedView()};
            }
        }

        [[nodiscard]]
        friend constexpr cReal auto operator*(cfg::cRealRep auto value, UnitIdentifier unitId) {
            return Real{ value, unitId };
        }

        [[nodiscard]]
        friend constexpr cReal auto operator/(cfg::cRealRep auto value, UnitIdentifier invUnitId) {
            return Real{ value, invUnitId.inverse() };
        }
    private:
        [[nodiscard]]
        static constexpr bool isSorted() {
            if constexpr (tailUnit().isOne()) {
                return true;
            } else {
                return headTerm() < tailUnit().headTerm();
            }
        }

        [[nodiscard]]
        static constexpr utils::cSizedString auto headText() {
            constexpr utils::cSizedString auto const& headSymbol = headTerm().basicUnit().symbol();
            constexpr cExponent auto headExp = headTerm().exponent();
            if constexpr (headExp == Exponent<1, 1>{}) {
                return headSymbol;
            }
            else {
                return utils::SizedString{ headSymbol.sizedView(), headExp.toString().sizedView()};
            }
        }

        static_assert(isSorted(), "Terms in UnitIdentifier must be sorted in ascending order.");
    };

    template<>
    class UnitIdentifier<> {
    public:
        [[nodiscard]]
        constexpr UnitIdentifier() = default;

        [[nodiscard]]
        static constexpr bool isOne() {
            return true;
        }

        [[nodiscard]]
        static constexpr UnitIdentifier<> inverse() {
            return {};
        }

        template<UnitTerm newHead>
        [[nodiscard]]
        static constexpr UnitIdentifier<newHead> addHead() {
            return {};
        }

        [[nodiscard]]
        static constexpr UnitIdentifier<> pow(cExponent auto) {
            return {};
        }

        [[nodiscard]]
        static constexpr utils::SizedString<char, 0> toString() {
            return {};
        }

        [[nodiscard]]
        friend constexpr cReal auto operator*(cfg::cRealRep auto value, UnitIdentifier unitId) {
            return Real{ value, unitId };
        }

        [[nodiscard]]
        friend constexpr cReal auto operator/(cfg::cRealRep auto value, UnitIdentifier invUnitId) {
            return Real{ value, invUnitId.inverse() };
        }
    };

    template<UnitTerm headTerm_, UnitTerm... tailTerms_>
    constexpr auto UnitIdentifier<headTerm_,tailTerms_...>::pow(cExponent auto exp) {
        if constexpr (exp.isZero()) {
            return UnitIdentifier<>{};
        } else {
            return UnitIdentifier<headTerm()* exp, tailTerms_* exp...>{};
        }
    }

    template<cBasicUnitIdentifier BasicId>
    [[nodiscard]]
    constexpr auto makeUnitIdentifier() {
        constexpr Exponent<1, 1> exp{};
        constexpr UnitTerm<BasicId{}, exp> term{};
        return UnitIdentifier<term>{};
    }

    [[nodiscard]]
    constexpr bool operator==(cUnitIdentifier auto lhs, cUnitIdentifier auto rhs) {
        return std::is_same_v<decltype(lhs), decltype(rhs)>;
    }

    [[nodiscard]]
    constexpr auto operator*(cUnitIdentifier auto lhs, cUnitIdentifier auto rhs) {
        if constexpr (lhs.isOne()) {
            return rhs;
        } else if constexpr (rhs.isOne()) {
            return lhs;
        } else {
            constexpr cUnitTerm auto lHead = lhs.headTerm();
            constexpr cUnitTerm auto rHead = rhs.headTerm();
            constexpr auto cmp = lHead <=> rHead;
            if constexpr (cmp == 0) {
                constexpr cUnitTerm auto headSum = lHead + rHead;
                constexpr cUnitIdentifier auto tails = lhs.tailUnit() * rhs.tailUnit();
                if constexpr (headSum.exponent().isZero()) {
                    return tails;
                } else {
                    return tails.template addHead<headSum>();
                }
            } else if constexpr (cmp < 0) {
                return (lhs.tailUnit() * rhs).template addHead<lHead>();
            } else {
                return (lhs * rhs.tailUnit()).template addHead<rHead>();
            }
        }
    }

    [[nodiscard]]
    constexpr auto operator/(cUnitIdentifier auto lhs, cUnitIdentifier auto rhs) {
        return lhs * rhs.inverse();
    }
}
