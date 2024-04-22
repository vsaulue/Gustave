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

#include <gustave/cfg/cRealOf.hpp>
#include <gustave/cfg/cRealRep.hpp>
#include <gustave/cfg/cRealTraits.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/math3d/cRealConstArg.hpp>
#include <gustave/math3d/Vector3.hpp>

namespace gustave::math3d {
    template<cfg::cRealTraits auto rt, cfg::cRealRep Rep_>
    class NormalizedVector3;

    template<typename T>
    concept cNormalizedVector3 = std::is_same_v<T, NormalizedVector3<T::realTraits(), typename T::Coord::Rep>>;

    namespace detail {
        template<cNormalizedVector3 T>
        struct AsVector3ConstArg<T> {
            [[nodiscard]]
            static constexpr auto const& convert(T const& normal) {
                return normal.value();
            }
        };
    }

    template<cfg::cRealTraits auto rt, cfg::cRealRep Rep_>
    class NormalizedVector3 {
    public:
        static constexpr auto one = rt.units().one;

        using RealTraits = decltype(rt);
        using Vector = Vector3<rt, one, Rep_>;
        using Coord = typename Vector::Coord;

        [[nodiscard]]
        static constexpr RealTraits realTraits() {
            return {};
        }

        [[nodiscard]]
        static NormalizedVector3 basisVector(BasicDirection direction) {
            NormalizedVector3 result{ UncheckedInit{}, 1.f, 0.f, 0.f };
            switch (direction.id()) {
            case BasicDirection::Id::plusX:
                result = { UncheckedInit{}, 1.f, 0.f, 0.f };
                break;
            case BasicDirection::Id::minusX:
                result = { UncheckedInit{}, -1.f, 0.f, 0.f };
                break;
            case BasicDirection::Id::plusY:
                result = { UncheckedInit{}, 0.f, 1.f, 0.f };
                break;
            case BasicDirection::Id::minusY:
                result = { UncheckedInit{}, 0.f, -1.f, 0.f };
                break;
            case BasicDirection::Id::plusZ:
                result = { UncheckedInit{}, 0.f, 0.f, 1.f };
                break;
            case BasicDirection::Id::minusZ:
                result = { UncheckedInit{}, 0.f, 0.f, -1.f };
                break;
            }
            return result;
        }

        [[nodiscard]]
        NormalizedVector3(Coord x, Coord y, Coord z)
            : value_{ normalize(Vector{ x, y, z }) }
        {

        }

        [[nodiscard]]
        explicit NormalizedVector3(cVector3 auto const& vector) :
            value_{ normalize(vector) }
        {

        }

        [[nodiscard]]
        NormalizedVector3 operator-() const {
            return { UncheckedInit{}, -x(), -y(), -z() };
        }

        [[nodiscard]]
        const Vector& value() const {
            return value_;
        }

        [[nodiscard]]
        cfg::cRealOf<rt> auto dot(cVector3ConstArg auto const& other) const {
            return value_.dot(other);
        }

        [[nodiscard]]
        constexpr cVector3 auto operator+(cVector3ConstArg auto const& rhs) const {
            return value_ + rhs;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator-(cVector3ConstArg auto const& rhs) const {
            return value_ - rhs;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator*(cRealConstArg<rt> auto const rhs) const {
            return value_ * rhs;
        }

        [[nodiscard]]
        friend constexpr cVector3 auto operator*(cRealConstArg<rt> auto const lhs, NormalizedVector3 const& rhs) {
            return lhs * rhs.value_;
        }

        [[nodiscard]]
        constexpr cVector3 auto operator/(cRealConstArg<rt> auto const rhs) const {
            return value_ / rhs;
        }

        [[nodiscard]]
        Coord const x() const {
            return value_.x();
        }

        [[nodiscard]]
        Coord const y() const {
            return value_.y();
        }

        [[nodiscard]]
        Coord const z() const {
            return value_.z();
        }

        [[nodiscard]]
        bool operator==(cVector3ConstArg auto const& rhs) const {
            return value_ == rhs;
        }
    private:
        Vector value_;

        [[nodiscard]]
        static Vector normalize(cVector3 auto const& other) {
            const cfg::cRealOf<rt> auto norm = other.norm();
            if (norm.value() < 0.00001) {
                throw std::domain_error("Cannot normalize: vector is almost zero.");
            }
            return other / norm;
        }

        struct UncheckedInit {};

        [[nodiscard]]
        NormalizedVector3(UncheckedInit, Coord x, Coord y, Coord z)
            : value_{x, y, z}
        {

        }
    };
}
