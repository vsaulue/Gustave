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

#include <algorithm>
#include <array>
#include <concepts>
#include <ostream>
#include <utility>

#include <gustave/cfg/cRealOf.hpp>
#include <gustave/cfg/cRealTraits.hpp>
#include <gustave/cfg/cUnitOf.hpp>

namespace Gustave::Math3d {
    // concept moved to `requires` for MSVC (https://developercommunity.visualstudio.com/t/Template-parameters:-constraints-dependi/10312655).
    template<Cfg::cRealTraits auto rt, auto unit_>
        requires Cfg::cUnitOf<decltype(unit_), rt>
    struct Vector3;

    template<typename T>
    concept cVector3 = std::same_as<T, Vector3<T::realTraits(), T::unit()>>;

    namespace detail {
        template<typename T>
        struct AsVector3ConstArg;

        template<cVector3 T>
        struct AsVector3ConstArg<T> {
            [[nodiscard]]
            static constexpr T const& convert(T const& vector) {
                return vector;
            }
        };
    }

    template<typename T>
    concept cVector3ConstArg = requires (T const& cv) {
        requires requires (detail::AsVector3ConstArg<std::remove_cvref_t<T>> const& converter) {
            { Meta::value(converter.convert(cv)) } -> cVector3;
        };
    };

    [[nodiscard]]
    constexpr auto const& asVector3ConstArg(cVector3ConstArg auto const& vector) {
        using Converter = detail::AsVector3ConstArg<decltype(Meta::value(vector))>;
        return Converter::convert(vector);
    }

    template<Cfg::cRealTraits auto rt, auto unit_>
        requires Cfg::cUnitOf<decltype(unit_),rt>
    struct Vector3 {
    private:
        using Unit = decltype(unit_);
        using RealTraits = decltype(rt);

        static constexpr auto one = rt.units().one;

        template<Cfg::cUnitOf<rt> auto u>
        using Real = typename RealTraits::template Type<u>;
    public:
        using Coord = Real<unit_>;
        using RealRep = typename Coord::Rep;

        [[nodiscard]]
        static constexpr Unit unit() {
            return {};
        }

        [[nodiscard]]
        static constexpr RealTraits realTraits() {
            return {};
        }

        [[nodiscard]]
        constexpr Vector3(Coord x, Coord y, Coord z) :
            values{x, y, z}
        {

        }

        [[nodiscard]]
        constexpr Vector3(RealRep x, RealRep y, RealRep z, Cfg::cUnitOf<rt> auto argUnit) :
            values{x * argUnit, y * argUnit, z * argUnit}
        {

        }

        [[nodiscard]]
        constexpr Vector3(cVector3 auto const& other)
            requires (!std::is_same_v<Vector3, decltype(Meta::value(other))>)
            : values{other.x(), other.y(), other.z()}
        {
            using Other = decltype(Meta::value(other));
            static_assert(rt == Other::realTraits(), "Invalid conversion: incompatible traits.");
            static_assert(isCompatible(Other::unit()), "Invalid conversion: incompatible units.");
        }

        constexpr Vector3& operator=(cVector3 auto const& other)
            requires (!std::is_same_v<Vector3,decltype(Meta::value(other))>)
        {
            using Other = decltype(Meta::value(other));
            static_assert(rt == Other::realTraits(), "Invalid conversion: incompatible traits.");
            static_assert(isCompatible(Other::unit()), "Invalid conversion: incompatible units.");
            std::ranges::copy(other.values, values.begin());
            return *this;
        }

        [[nodiscard]]
        static constexpr Vector3 zero() {
            constexpr Coord cZero = Coord::zero();
            return { cZero, cZero, cZero };
        }

        [[nodiscard]]
        constexpr Vector3 operator-() const {
            return { -x(), -y(), -z() };
        }

        template<auto rhsUnit> // gcc bug 109160: concept moved into `requires`.
            requires (RealTraits::isUnit(rhsUnit))
        constexpr Vector3& operator+=(Vector3<rt, rhsUnit> const& rhs) {
            using Rhs = decltype(Meta::value(rhs));
            static_assert(isCompatible(Rhs::unit()), "Invalid addition: incompatible units.");
            for (std::size_t i = 0; i < values.size(); ++i) {
                values[i] += rhs.values[i];
            }
            return *this;
        }

        template<auto rhsUnit> // gcc bug 109160: concept moved into `requires`.
            requires (RealTraits::isUnit(rhsUnit))
        constexpr Vector3 operator+(Vector3<rt, rhsUnit> const& rhs) const {
            using Rhs = decltype(Meta::value(rhs));
            static_assert(isCompatible(Rhs::unit()), "Invalid addition: incompatible units.");
            Vector3 result = *this;
            result += rhs;
            return result;
        }

        template<auto rhsUnit> // gcc bug 109160: concept moved into `requires`.
            requires (RealTraits::isUnit(rhsUnit))
        constexpr Vector3& operator-=(Vector3<rt, rhsUnit> const& rhs) {
            using Rhs = decltype(Meta::value(rhs));
            static_assert(isCompatible(Rhs::unit()), "Invalid substraction: incompatible units.");
            for (std::size_t i = 0; i < values.size(); ++i) {
                values[i] -= rhs.values[i];
            }
            return *this;
        }

        template<auto rhsUnit> // gcc bug 109160: concept moved into `requires`.
            requires (RealTraits::isUnit(rhsUnit))
        constexpr Vector3 operator-(Vector3<rt, rhsUnit> const& rhs) const {
            using Rhs = decltype(Meta::value(rhs));
            static_assert(isCompatible(Rhs::unit()), "Invalid addition: incompatible units.");
            Vector3 result = *this;
            result -= rhs;
            return result;
        }

        constexpr Vector3& operator*=(Real<one> rhs) {
            for (auto& value : values) {
                value *= rhs;
            }
            return *this;
        }

        constexpr Vector3& operator/=(Real<one> rhs) {
            for (auto& value : values) {
                value /= rhs;
            }
            return *this;
        }

        [[nodiscard]]
        constexpr Coord& x() {
            return values[0];
        }

        [[nodiscard]]
        constexpr Coord& y() {
            return values[1];
        }

        [[nodiscard]]
        constexpr Coord& z() {
            return values[2];
        }

        [[nodiscard]]
        constexpr Coord x() const {
            return values[0];
        }

        [[nodiscard]]
        constexpr Coord y() const {
            return values[1];
        }

        [[nodiscard]]
        constexpr Coord z() const {
            return values[2];
        }

        [[nodiscard]]
        Coord norm() const {
            using T2 = Real<unit()* unit()>;
            auto result = T2::zero();
            for (const auto& value : values) {
                result = result + value * value;
            }
            return RealTraits::sqrt(result);
        }

        [[nodiscard]]
        constexpr auto dot(cVector3ConstArg auto const& other) const {
            cVector3 auto const& otherV3 = asVector3ConstArg(other);
            using OtherV3 = decltype(Meta::value(otherV3));
            static_assert(rt == OtherV3::realTraits(), "Invalid conversion: incompatible traits.");
            auto result = Real<unit() * OtherV3::unit()>::zero();
            for (unsigned i = 0; i < values.size(); ++i) {
                result += values[i] * otherV3.values[i];
            }
            return result;
        }

        [[nodiscard]]
        static constexpr bool isCompatible(Cfg::cUnitOf<rt> auto otherUnit) {
            return unit().isAssignableFrom(otherUnit);
        }

        std::array<Coord, 3> values;
    };

    namespace detail {
        template<Cfg::cRealTraits Traits>
        [[nodiscard]]
        constexpr auto asReal(Traits, std::floating_point auto value) {
            constexpr auto one = Traits::units().one;
            using Real = typename Traits::template Type<one>;
            return Real(value);
        }

        template<Cfg::cRealTraits Traits>
        [[nodiscard]]
        constexpr auto asReal(Traits, Cfg::cRealOf<Traits{}> auto value) {
            return value;
        }
    }

    [[nodiscard]]
    constexpr cVector3 auto operator*(auto lhs, cVector3 auto const& rhs)
        requires requires { detail::asReal(rhs.realTraits(), lhs); }
    {
        using Rhs = decltype(Meta::value(rhs));
        constexpr auto traits = Rhs::realTraits();
        const auto l = detail::asReal(traits, lhs);
        using L = decltype(l);
        constexpr auto unit = L::unit() * Rhs::unit();
        return Vector3<traits, unit>{l * rhs.x(), l * rhs.y(), l * rhs.z() };
    }

    [[nodiscard]]
    constexpr cVector3 auto operator/(cVector3 auto const& lhs, auto rhs)
        requires requires { detail::asReal(lhs.realTraits(), rhs); }
    {
        using Lhs = decltype(Meta::value(lhs));
        constexpr auto traits = Lhs::realTraits();
        const auto r = detail::asReal(traits, rhs);
        using R = decltype(r);
        constexpr auto unit = Lhs::unit() / R::unit();
        return Vector3<traits, unit>{ lhs.x() / r, lhs.y() / r, lhs.z() / r};
    }

    [[nodiscard]]
    constexpr bool operator==(cVector3 auto const& lhs, cVector3 auto const& rhs) {
        using Lhs = decltype(Meta::value(lhs));
        using Rhs = decltype(Meta::value(rhs));
        static_assert(Lhs::realTraits() == Rhs::realTraits(), "Invalid comparison: different realTraits.");
        static_assert(Lhs::isCompatible(Rhs::unit()), "Invalid comparison: incompatible units.");
        return std::ranges::equal(lhs.values, rhs.values);
    }

    std::ostream& operator<<(std::ostream& stream, cVector3 auto const& vector) {
        return stream << '{' << vector.x().value() << ", " << vector.y().value() << ", " << vector.z().value() << "}" << vector.unit();
    }
}
