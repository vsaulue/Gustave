/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <type_traits>

#include <gustave/units/lib/Unit.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Units::Lib {
    template<typename T>
    concept cRealRep = std::floating_point<T> && !std::is_const_v<T>;

    template<Unit unit_, cRealRep Rep_>
    class Real;

    template<typename T>
    concept cReal = std::same_as<T, Real<T::unit(), typename T::Rep>>;

    template<Unit unit_, cRealRep Rep_>
    class Real {
    public:
        using Rep = Rep_;

        [[nodiscard]]
        explicit constexpr Real(Utils::NoInit) {}

        [[nodiscard]]
        constexpr Real(Rep value)
            requires (unit_.isTrivialOne())
        :
            value_(value)
        {

        }

        [[nodiscard]]
        explicit constexpr Real(Rep value, cUnit auto u)
            : value_{ value }
        {
            static_assert(isCompatible(u), "Invalid conversion: incompatible units.");
        }

        [[nodiscard]]
        constexpr Real(cReal auto other)
            : value_(other.value())
        {
            using Other = decltype(other);
            static_assert(Detail::template isNotNarrowingFrom<Other>(), "Invalid conversion: narrowing representation");
            static_assert(isCompatible(other.unit()), "Invalid conversion: incompatible units");
        }

        constexpr Real& operator=(cReal auto other) {
            using Other = decltype(other);
            static_assert(Detail::template isNotNarrowingFrom<Other>(), "Invalid conversion: narrowing representation");
            static_assert(isCompatible(other.unit()), "Invalid conversion: incompatible units");
            value() = other.value();
            return *this;
        }

        [[nodiscard]]
        static constexpr auto unit() {
            return unit_;
        }

        [[nodiscard]]
        static constexpr Real zero() {
            return Real{ 0.f, unit_ };
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
        constexpr Real operator-() const {
            return Real{ -value_, unit_ };
        }

        constexpr Real& operator+=(cReal auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(isCompatible(rhs.unit()), "Invalid addition: incompatible units.");
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid addition: narrowing conversion.");
            this->value() += rhs.value();
            return *this;
        }

        constexpr Real& operator+=(std::floating_point auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid addition: narrowing conversion.");
            static_assert(unit().isOne(), "Invalid addition: raw float can only be added to a Real of dimension one.");
            this->value() += rhs;
            return *this;
        }

        constexpr Real& operator-=(cReal auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(isCompatible(rhs.unit()), "Invalid substraction: incompatible units.");
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid substraction: narrowing conversion.");
            this->value() -= rhs.value();
            return *this;
        }

        constexpr Real& operator-=(std::floating_point auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid substraction: narrowing conversion.");
            static_assert(unit().isOne(), "Invalid substraction: raw float can only be added to a Real of dimension one.");
            this->value() -= rhs;
            return *this;
        }

        constexpr Real& operator*=(cReal auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid multiplication: narrowing conversion.");
            static_assert(rhs.unit().isOne(), "Invalid multiplication: right-hand side must be of dimension one.");
            this->value() *= rhs.value();
            return *this;
        }

        constexpr Real& operator*=(std::floating_point auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid multiplication: narrowing conversion.");
            this->value() *= rhs;
            return *this;
        }

        constexpr Real& operator/=(cReal auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid division: narrowing conversion.");
            static_assert(rhs.unit().isOne(), "Invalid division: right-hand side must be of dimension one.");
            this->value() /= rhs.value();
            return *this;
        }

        constexpr Real& operator/=(std::floating_point auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Detail::template isNotNarrowingFrom<Rhs>(), "Invalid division: narrowing conversion.");
            this->value() /= rhs;
            return *this;
        }

        struct Detail {
            template<std::floating_point SourceRep>
            static constexpr bool isNotNarrowingFrom() {
                return sizeof(Rep) >= sizeof(SourceRep);
            }

            template<cReal Source>
            static constexpr bool isNotNarrowingFrom() {
                return isNotNarrowingFrom<typename Source::Rep>();
            }
        };

        [[nodiscard]]
        static constexpr bool isCompatible(cUnit auto otherUnit) {
            return unit().isAssignableFrom(otherUnit);
        }

        static constexpr Detail detail{};
    private:
        Rep value_;
    };

    // constructors

    [[nodiscard]]
    constexpr cReal auto operator*(std::floating_point auto value, cUnit auto unit) {
        using Rep = decltype(value);
        using ArgUnit = decltype(unit);
        using Res = Real<ArgUnit{}, Rep>;
        return Res{ value, unit };
    }

    [[nodiscard]]
    constexpr cReal auto operator*(std::floating_point auto value, cUnitIdentifier auto unitId) {
        return value * Unit(unitId);
    }

    [[nodiscard]]
    constexpr cReal auto operator/(std::floating_point auto value, cUnit auto invUnit) {
        return value * invUnit.inverse();
    }

    [[nodiscard]]
    constexpr cReal auto operator/(std::floating_point auto value, cUnitIdentifier auto invUnitId) {
        return value * invUnitId.inverse();
    }

    // addition

    [[nodiscard]]
    constexpr cReal auto operator+(cReal auto lhs, cReal auto rhs) {
        static_assert(lhs.isCompatible(rhs.unit()), "Invalid addition: incompatible units.");
        return (lhs.value() + rhs.value()) * lhs.unit();
    }

    [[nodiscard]]
    constexpr cReal auto operator+(cReal auto lhs, std::floating_point auto rhs) {
        static_assert(lhs.unit().isOne(), "Invalid addition: (Real with non-one dimension) + (raw float)");
        return (lhs.value() + rhs) * lhs.unit();
    }

    [[nodiscard]]
    constexpr cReal auto operator+(std::floating_point auto lhs, cReal auto rhs) {
        static_assert(rhs.unit().isOne(), "Invalid addition: (raw float) + (Real with non-one dimension)");
        return (lhs + rhs.value()) * rhs.unit();
    }

    // substraction

    [[nodiscard]]
    constexpr cReal auto operator-(cReal auto lhs, cReal auto rhs) {
        static_assert(lhs.isCompatible(rhs.unit()), "Invalid substraction: incompatible units.");
        return (lhs.value() - rhs.value()) * lhs.unit();
    }

    [[nodiscard]]
    constexpr cReal auto operator-(cReal auto lhs, std::floating_point auto rhs) {
        static_assert(lhs.unit().isOne(), "Invalid substraction: (Real with non-one dimension) + (raw float)");
        return (lhs.value() - rhs) * lhs.unit();
    }

    [[nodiscard]]
    constexpr cReal auto operator-(std::floating_point auto lhs, cReal auto rhs) {
        static_assert(rhs.unit().isOne(), "Invalid substraction: (raw float) + (Real with non-one dimension)");
        return (lhs - rhs.value()) * rhs.unit();
    }

    // multiplication

    [[nodiscard]]
    constexpr cReal auto operator*(cReal auto lhs, cReal auto rhs) {
        return (lhs.value() * rhs.value()) * (lhs.unit() * rhs.unit());
    }

    [[nodiscard]]
    constexpr cReal auto operator*(cReal auto lhs, std::floating_point auto rhs) {
        return (lhs.value() * rhs) * lhs.unit();
    }

    [[nodiscard]]
    constexpr cReal auto operator*(std::floating_point auto lhs, cReal auto rhs) {
        return (lhs * rhs.value()) * rhs.unit();
    }

    // division

    [[nodiscard]]
    constexpr cReal auto operator/(cReal auto lhs, cReal auto rhs) {
        return (lhs.value() / rhs.value()) * (lhs.unit() / rhs.unit());
    }

    [[nodiscard]]
    constexpr cReal auto operator/(cReal auto lhs, std::floating_point auto rhs) {
        return (lhs.value() / rhs) * lhs.unit();
    }

    [[nodiscard]]
    constexpr cReal auto operator/(std::floating_point auto lhs, cReal auto rhs) {
        return (lhs / rhs.value()) / rhs.unit();
    }

    // equality

    [[nodiscard]]
    constexpr bool operator==(cReal auto lhs, cReal auto rhs) {
        static_assert(lhs.isCompatible(rhs.unit()), "Invalid comparison: incompatible units.");
        return lhs.value() == rhs.value();
    }

    [[nodiscard]]
    constexpr bool operator==(cReal auto lhs, std::floating_point auto rhs) {
        static_assert(lhs.unit().isOne(), "Invalid comparison: (Real with non-one dimension) == (raw float).");
        return lhs.value() == rhs;
    }

    [[nodiscard]]
    constexpr bool operator==(std::floating_point auto lhs, cReal auto rhs) {
        static_assert(rhs.unit().isOne(), "Invalid comparison: (raw float) == (Real with non-one dimension).");
        return lhs == rhs.value();
    }

    // comparison

    [[nodiscard]]
    constexpr std::partial_ordering operator<=>(cReal auto lhs, cReal auto rhs) {
        static_assert(lhs.isCompatible(rhs.unit()), "Invalid comparison: incompatible units.");
        return lhs.value() <=> rhs.value();
    }

    [[nodiscard]]
    constexpr std::partial_ordering operator<=>(cReal auto lhs, std::floating_point auto rhs) {
        static_assert(lhs.unit().isOne(), "Invalid comparison: (Real with non-one dimension) == (raw float).");
        return lhs.value() <=> rhs;
    }

    [[nodiscard]]
    constexpr std::partial_ordering operator<=>(std::floating_point auto lhs, cReal auto rhs) {
        static_assert(rhs.unit().isOne(), "Invalid comparison: (raw float) == (Real with non-one dimension).");
        return lhs <=> rhs.value();
    }

    // printing

    std::ostream& operator<<(std::ostream& stream, cReal auto real) {
        return stream << real.value() << ' ' << real.unit();
    }
}
