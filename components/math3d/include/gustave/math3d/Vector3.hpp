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

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>
#include <ostream>
#include <utility>

#include <gustave/cfg/cRealOf.hpp>
#include <gustave/cfg/cRealTraits.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/math3d/cRealConstArg.hpp>

namespace gustave::math3d {
    // concept moved to `requires` for MSVC (https://developercommunity.visualstudio.com/t/Template-parameters:-constraints-dependi/10312655).
    template<cfg::cRealTraits auto rt, auto unit_, cfg::cRealRep Rep_>
        requires cfg::cUnitOf<decltype(unit_), rt>
    class Vector3;

    template<typename T>
    concept cVector3 = std::same_as<T, Vector3<T::realTraits(), T::unit(), typename T::RealRep>>;

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
            { meta::value(converter.convert(cv)) } -> cVector3;
        };
    };

    [[nodiscard]]
    constexpr auto const& asVector3ConstArg(cVector3ConstArg auto const& vector) {
        using Converter = detail::AsVector3ConstArg<decltype(meta::value(vector))>;
        return Converter::convert(vector);
    }

    namespace detail {
        template<cfg::cRealTraits Traits, cfg::cRealOf<Traits{}> Real>
        [[nodiscard]]
        constexpr cVector3 auto vector3(Traits, Real x, Real y, Real z) {
            return Vector3<Traits{}, Real::unit(), typename Real::Rep>{x, y, z};
        }
    }

    template<cfg::cRealTraits auto rt, auto unit_, cfg::cRealRep Rep_>
        requires cfg::cUnitOf<decltype(unit_),rt>
    class Vector3 {
    private:
        using Unit = decltype(unit_);
        using RealTraits = decltype(rt);

        static constexpr auto one = rt.units().one;

        template<cfg::cUnitOf<rt> auto u, cfg::cRealRep R>
        using Real = typename RealTraits::template Type<u,R>;
    public:
        using RealRep = Rep_;
        using Coord = Real<unit_, Rep_>;

        [[nodiscard]]
        static constexpr Unit unit() {
            return {};
        }

        [[nodiscard]]
        static constexpr RealTraits realTraits() {
            return {};
        }

        [[nodiscard]]
        explicit constexpr Vector3(utils::NoInit NO_INIT)
            : coords_{ Coord{NO_INIT}, Coord{NO_INIT}, Coord{NO_INIT} }
        {}

        [[nodiscard]]
        constexpr Vector3(Coord x, Coord y, Coord z) :
            coords_{x, y, z}
        {

        }

        [[nodiscard]]
        constexpr Vector3(RealRep x, RealRep y, RealRep z, cfg::cUnitOf<rt> auto argUnit) :
            coords_{x * argUnit, y * argUnit, z * argUnit}
        {

        }

        [[nodiscard]]
        constexpr Vector3(cVector3ConstArg auto const& other)
            requires (!std::is_same_v<Vector3, decltype(meta::value(other))>)
            : Vector3{ utils::NO_INIT }
        {
            auto const& otherV3 = asVector3ConstArg(other);
            using OtherV3 = decltype(meta::value(otherV3));
            static_assert(isNotNarrowingFrom<typename OtherV3::Coord>(), "Invalid conversion: narrowing conversion.");
            static_assert(rt == OtherV3::realTraits(), "Invalid conversion: incompatible traits.");
            static_assert(isCompatible(OtherV3::unit()), "Invalid conversion: incompatible units.");
            std::ranges::copy(otherV3.coords(), coords_.begin());
        }

        constexpr Vector3& operator=(cVector3ConstArg auto const& other)
            requires (!std::is_same_v<Vector3,decltype(meta::value(other))>)
        {
            auto const& otherV3 = asVector3ConstArg(other);
            using OtherV3 = decltype(meta::value(otherV3));
            static_assert(isNotNarrowingFrom<typename OtherV3::Coord>(), "Invalid conversion: narrowing conversion.");
            static_assert(rt == OtherV3::realTraits(), "Invalid conversion: incompatible traits.");
            static_assert(isCompatible(OtherV3::unit()), "Invalid conversion: incompatible units.");
            std::ranges::copy(otherV3.coords(), coords_.begin());
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

        constexpr Vector3& operator+=(cVector3ConstArg auto const& rhs) {
            cVector3 auto const& rhsV3 = asVector3ConstArg(rhs);
            using RhsV3 = decltype(meta::value(rhsV3));
            static_assert(isNotNarrowingFrom<typename RhsV3::Coord>(), "Invalid addition: narrowing conversion.");
            static_assert(rt == RhsV3::realTraits(), "Invalid addition: incompatible traits.");
            static_assert(isCompatible(RhsV3::unit()), "Invalid addition: incompatible units.");
            for (std::size_t i = 0; i < coords_.size(); ++i) {
                coords_[i] += rhsV3.coords()[i];
            }
            return *this;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator+(cVector3ConstArg auto const& rhs) const {
            cVector3 auto const& rhsV3 = asVector3ConstArg(rhs);
            using RhsV3 = decltype(meta::value(rhsV3));
            static_assert(rt == RhsV3::realTraits(), "Invalid addition: incompatible traits.");
            static_assert(isCompatible(RhsV3::unit()), "Invalid addition: incompatible units.");
            using ResRep = std::common_type_t<Rep_, typename RhsV3::Coord::Rep>;
            Vector3<rt, unit_, ResRep> result = *this;
            for (std::size_t i = 0; i < coords_.size(); ++i) {
                result.coords()[i] += rhsV3.coords()[i];
            }
            return result;
        }

        constexpr Vector3& operator-=(cVector3ConstArg auto const& rhs) {
            cVector3 auto const& rhsV3 = asVector3ConstArg(rhs);
            using RhsV3 = decltype(meta::value(rhsV3));
            static_assert(isNotNarrowingFrom<typename RhsV3::Coord>(), "Invalid subtraction: narrowing conversion.");
            static_assert(rt == RhsV3::realTraits(), "Invalid subtraction: incompatible traits.");
            static_assert(isCompatible(RhsV3::unit()), "Invalid subtraction: incompatible units.");
            for (std::size_t i = 0; i < coords_.size(); ++i) {
                coords_[i] -= rhsV3.coords()[i];
            }
            return *this;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator-(cVector3ConstArg auto const& rhs) const {
            cVector3 auto const& rhsV3 = asVector3ConstArg(rhs);
            using RhsV3 = decltype(meta::value(rhsV3));
            static_assert(rt == RhsV3::realTraits(), "Invalid subtraction: incompatible traits.");
            static_assert(isCompatible(RhsV3::unit()), "Invalid subtraction: incompatible units.");
            using ResUnit = std::common_type_t<Rep_, typename RhsV3::Coord::Rep>;
            Vector3<rt, unit_, ResUnit> result = *this;
            for (std::size_t i = 0; i < coords_.size(); ++i) {
                result.coords()[i] -= rhsV3.coords()[i];
            }
            return result;
        }

        constexpr Vector3& operator*=(cRealConstArg<rt> auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Rhs::unit().isOne(), "Invalid multiplication: second operand must be of dimension one.");
            static_assert(isNotNarrowingFrom<Rhs>, "Invalid multiplication: narrowing conversion.");
            for (auto& coord : coords_) {
                coord *= rhs;
            }
            return *this;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator*(cRealConstArg<rt> auto const rhs) const {
            return detail::vector3(rt, coords_[0] * rhs, coords_[1] * rhs, coords_[2] * rhs);
        }

        [[nodiscard]]
        friend constexpr cVector3 auto operator*(cRealConstArg<rt> auto const lhs, Vector3 const& rhs) {
            auto const& coords = rhs.coords();
            return detail::vector3(rt, lhs * coords[0], lhs * coords[1], lhs * coords[2]);
        }

        constexpr Vector3& operator/=(cRealConstArg<rt> auto rhs) {
            using Rhs = decltype(rhs);
            static_assert(Rhs::unit().isOne(), "Invalid division: second operand must be of dimension one.");
            static_assert(isNotNarrowingFrom<Rhs>, "Invalid division: narrowing conversion.");
            for (auto& coord : coords_) {
                coord /= rhs;
            }
            return *this;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator/(cRealConstArg<rt> auto const rhs) const {
            return detail::vector3(rt, coords_[0] / rhs, coords_[1] / rhs, coords_[2] / rhs);
        }

        [[nodiscard]]
        constexpr std::array<Coord, 3>& coords() {
            return coords_;
        }

        [[nodiscard]]
        constexpr std::array<Coord, 3> const& coords() const {
            return coords_;
        }

        [[nodiscard]]
        constexpr Coord& x() {
            return coords_[0];
        }

        [[nodiscard]]
        constexpr Coord& y() {
            return coords_[1];
        }

        [[nodiscard]]
        constexpr Coord& z() {
            return coords_[2];
        }

        [[nodiscard]]
        constexpr Coord const x() const {
            return coords_[0];
        }

        [[nodiscard]]
        constexpr Coord const y() const {
            return coords_[1];
        }

        [[nodiscard]]
        constexpr Coord const z() const {
            return coords_[2];
        }

        [[nodiscard]]
        Coord norm() const {
            using T2 = Real<unit() * unit(), RealRep>;
            auto result = T2::zero();
            for (auto const coord : coords_) {
                result = result + coord * coord;
            }
            return RealTraits::sqrt(result);
        }

        [[nodiscard]]
        constexpr auto dot(cVector3ConstArg auto const& other) const {
            cVector3 auto const& otherV3 = asVector3ConstArg(other);
            using OtherV3 = decltype(meta::value(otherV3));
            static_assert(rt == OtherV3::realTraits(), "Invalid conversion: incompatible traits.");
            using ResRep = std::common_type_t<typename Coord::Rep, typename OtherV3::Coord::Rep>;
            auto result = Real<unit() * OtherV3::unit(), ResRep>::zero();
            for (unsigned i = 0; i < coords_.size(); ++i) {
                result += coords_[i] * otherV3.coords()[i];
            }
            return result;
        }

        [[nodiscard]]
        static constexpr bool isCompatible(cfg::cUnitOf<rt> auto otherUnit) {
            return unit().isAssignableFrom(otherUnit);
        }

        template<cRealConstArg<rt> Source>
        [[nodiscard]]
        static constexpr bool isNotNarrowingFrom() {
            return Coord::template isNotNarrowingFrom<Source>();
        }

        friend std::ostream& operator<<(std::ostream& stream, Vector3 const& vector) {
            return stream << '{' << vector.x().value() << ", " << vector.y().value() << ", " << vector.z().value() << "}" << vector.unit();
        }

        [[nodiscard]]
        constexpr bool operator==(cVector3ConstArg auto const& rhs) const {
            cVector3 auto const& rhsV3 = asVector3ConstArg(rhs);
            using RhsV3 = decltype(meta::value(rhsV3));
            static_assert(rt == RhsV3::realTraits(), "Invalid comparison: different realTraits.");
            static_assert(isCompatible(RhsV3::unit()), "Invalid comparison: incompatible units.");
            return std::ranges::equal(coords_, rhsV3.coords(), std::equal_to{});
        }
    private:
        std::array<Coord, 3> coords_;
    };
}
