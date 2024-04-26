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

#include <compare>
#include <concepts>
#include <ostream>

#include <gustave/units/stdUnitless/lib/Unit.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::units::stdUnitless::lib {
    template<cfg::cRealRep Rep_>
    class Real;

    template<typename T>
    concept cReal = std::same_as<T, Real<typename T::Rep>>;

    template<cfg::cRealRep Rep_>
    class Real {
    public:
        using Rep = Rep_;

        [[nodiscard]]
        explicit constexpr Real(utils::NoInit)
            : value_{ std::numeric_limits<Rep>::signaling_NaN() }
        {}

        [[nodiscard]]
        constexpr Real(std::floating_point auto value)
            : value_{ value }
        {
            static_assert(isNotNarrowingFrom<decltype(value)>(), "Invalid conversion: narrowing representation.");
        }

        [[nodiscard]]
        constexpr Real(cReal auto other)
            : value_{ other.value() }
        {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid conversion: narrowing representation.");
        }

        [[nodiscard]]
        constexpr Real(std::floating_point auto value, Unit)
            : value_{ value }
        {
            static_assert(isNotNarrowingFrom<decltype(value)>(), "Invalid conversion: narrowing representation.");
        }

        Real& operator=(std::floating_point auto value) {
            static_assert(isNotNarrowingFrom<decltype(value)>(), "Invalid conversion: narrowing representation.");
            value_ = value;
            return *this;
        }

        Real& operator=(cReal auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid conversion: narrowing representation.");
            value_ = other.value();
            return *this;
        }

        [[nodiscard]]
        static constexpr bool isCompatible(Unit) {
            return true;
        }

        template<std::floating_point SourceRep>
        [[nodiscard]]
        static constexpr bool isNotNarrowingFrom() {
            return sizeof(Rep) >= sizeof(SourceRep);
        }

        template<cReal SourceReal>
        [[nodiscard]]
        static constexpr bool isNotNarrowingFrom() {
            return isNotNarrowingFrom<typename SourceReal::Rep>();
        }

        [[nodiscard]]
        static constexpr Unit unit() {
            return {};
        }

        [[nodiscard]]
        constexpr Rep& value() {
            return value_;
        }

        [[nodiscard]]
        constexpr const Rep& value() const {
            return value_;
        }

        [[nodiscard]]
        static constexpr Real zero() {
            return Real{ 0.f };
        }

        [[nodiscard]]
        constexpr operator Rep() {
            return value_;
        }

        friend std::ostream& operator<<(std::ostream& stream, Real real) {
            return stream << real.value_;
        }

        [[nodiscard]]
        constexpr Real operator-() const {
            return Real{ -value_ };
        }

        // operator += -= *= /=

        constexpr Real& operator+=(cReal auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid addition: narrowing representation.");
            value_ += other.value();
            return *this;
        }

        constexpr Real& operator+=(std::floating_point auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid addition: narrowing representation.");
            value_ += other;
            return *this;
        }

        constexpr Real& operator-=(cReal auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid subtraction: narrowing representation.");
            value_ -= other.value();
            return *this;
        }

        constexpr Real& operator-=(std::floating_point auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid subtraction: narrowing representation.");
            value_ -= other;
            return *this;
        }

        constexpr Real& operator*=(cReal auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid multiplication: narrowing representation.");
            value_ *= other.value();
            return *this;
        }

        constexpr Real& operator*=(std::floating_point auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid multiplication: narrowing representation.");
            value_ *= other;
            return *this;
        }

        constexpr Real& operator/=(cReal auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid division: narrowing representation.");
            value_ /= other.value();
            return *this;
        }

        constexpr Real& operator/=(std::floating_point auto other) {
            static_assert(isNotNarrowingFrom<decltype(other)>(), "Invalid division: narrowing representation.");
            value_ /= other;
            return *this;
        }

        // addition

        [[nodiscard]]
        constexpr cReal auto operator+(cReal auto other) const {
            using ResRep = std::common_type_t<Rep, typename decltype(other)::Rep>;
            return Real<ResRep>{ value_ + other.value() };
        }

        [[nodiscard]]
        constexpr cReal auto operator+(std::floating_point auto other) const {
            using ResRep = std::common_type_t<Rep, decltype(other)>;
            return Real<ResRep>{ value_ + other };
        }

        [[nodiscard]]
        friend constexpr cReal auto operator+(std::floating_point auto lhs, Real rhs) {
            using ResRep = std::common_type_t<decltype(lhs), Rep>;
            return Real<ResRep>{ lhs + rhs.value_ };
        }

        // subtraction

        [[nodiscard]]
        constexpr cReal auto operator-(cReal auto other) const {
            using ResRep = std::common_type_t<Rep, typename decltype(other)::Rep>;
            return Real<ResRep>{ value_ - other.value() };
        }

        [[nodiscard]]
        constexpr cReal auto operator-(std::floating_point auto other) const {
            using ResRep = std::common_type_t<Rep, decltype(other)>;
            return Real<ResRep>{ value_ - other };
        }

        [[nodiscard]]
        friend constexpr cReal auto operator-(std::floating_point auto lhs, Real rhs) {
            using ResRep = std::common_type_t<decltype(lhs), Rep>;
            return Real<ResRep>{ lhs - rhs.value_ };
        }

        // multiplication

        [[nodiscard]]
        constexpr cReal auto operator*(cReal auto other) const {
            using ResRep = std::common_type_t<Rep, typename decltype(other)::Rep>;
            return Real<ResRep>{ value_* other.value() };
        }

        [[nodiscard]]
        constexpr cReal auto operator*(std::floating_point auto other) const {
            using ResRep = std::common_type_t<Rep, decltype(other)>;
            return Real<ResRep>{ value_* other };
        }

        [[nodiscard]]
        friend constexpr cReal auto operator*(std::floating_point auto lhs, Real rhs) {
            using ResRep = std::common_type_t<decltype(lhs), Rep>;
            return Real<ResRep>{ lhs* rhs.value_ };
        }

        // division

        [[nodiscard]]
        constexpr cReal auto operator/(cReal auto other) const {
            using ResRep = std::common_type_t<Rep, typename decltype(other)::Rep>;
            return Real<ResRep>{ value_ / other.value() };
        }

        [[nodiscard]]
        constexpr cReal auto operator/(std::floating_point auto other) const {
            using ResRep = std::common_type_t<Rep, decltype(other)>;
            return Real<ResRep>{ value_ / other };
        }

        [[nodiscard]]
        friend constexpr cReal auto operator/(std::floating_point auto lhs, Real rhs) {
            using ResRep = std::common_type_t<decltype(lhs), Rep>;
            return Real<ResRep>{ lhs / rhs.value_ };
        }

        // equality

        [[nodiscard]]
        constexpr bool operator==(cReal auto other) const {
            return value_ == other.value();
        }

        [[nodiscard]]
        constexpr bool operator==(std::floating_point auto other) const {
            return value_ == other;
        }

        // comparison

        [[nodiscard]]
        constexpr auto operator<=>(cReal auto other) const {
            return value_ <=> other.value();
        }

        [[nodiscard]]
        constexpr auto operator<=>(std::floating_point auto other) const {
            return value_ <=> other;
        }
    private:
        Rep value_;
    };

    template<cfg::cRealRep Rep>
    Real(Rep) -> Real<Rep>;
}
