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

#include <type_traits>

#include <gustave/cfg/cRealOf.hpp>
#include <gustave/cfg/cRealTraits.hpp>
#include <gustave/math/BasicDirection.hpp>
#include <gustave/math/Vector3.hpp>

namespace Gustave::Math {
    template<Cfg::cRealTraits auto rt>
    class NormalizedVector3 {
    public:
        static constexpr auto one = rt.units().one;

        using RealTraits = decltype(rt);
        using Vector = Vector3<rt, one>;
        using Coord = typename Vector::Coord;

        [[nodiscard]]
        static constexpr RealTraits realTraits() {
            return {};
        }

        [[nodiscard]]
        static NormalizedVector3 basisVector(BasicDirection direction) {
            NormalizedVector3 result{ UncheckedInit{}, 1.f, 0.f, 0.f };
            switch (direction) {
            case BasicDirection::plusX:
                result = { UncheckedInit{}, 1.f, 0.f, 0.f };
                break;
            case BasicDirection::minusX:
                result = { UncheckedInit{}, -1.f, 0.f, 0.f };
                break;
            case BasicDirection::plusY:
                result = { UncheckedInit{}, 0.f, 1.f, 0.f };
                break;
            case BasicDirection::minusY:
                result = { UncheckedInit{}, 0.f, -1.f, 0.f };
                break;
            case BasicDirection::plusZ:
                result = { UncheckedInit{}, 0.f, 0.f, 1.f };
                break;
            case BasicDirection::minusZ:
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
        operator const Vector&() const {
            return value_;
        }

        [[nodiscard]]
        Coord dot(const Vector& other) const {
            return value_.dot(other);
        }

        [[nodiscard]]
        Coord x() const {
            return value_.x();
        }

        [[nodiscard]]
        Coord y() const {
            return value_.y();
        }

        [[nodiscard]]
        Coord z() const {
            return value_.z();
        }
    private:
        Vector value_;

        [[nodiscard]]
        static Vector normalize(cVector3 auto const& other) {
            const Cfg::cRealOf<rt> auto norm = other.norm();
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

    template<typename T>
    concept cNormalizedVector3 = std::is_same_v<T, NormalizedVector3<T::realTraits()>>;

    [[nodiscard]]
    constexpr cVector3 auto operator*(auto const& lhs, cNormalizedVector3 auto const& rhs)
        requires requires { lhs * rhs.value(); }
    {
        return lhs * rhs.value();
    }
}
