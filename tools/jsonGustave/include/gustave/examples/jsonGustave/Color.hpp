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

#include <concepts>
#include <cstdint>
#include <format>
#include <string>

#include <gustave/cfg/cRealRep.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave {
    template<cfg::cRealRep Coord_>
    class Color;

    template<typename T>
    concept cColor = std::same_as<T, Color<typename T::Coord>>;

    template<cfg::cRealRep Coord_>
    class Color {
    public:
        using Coord = Coord_;
        [[nodiscard]]
        Color(Coord r, Coord g, Coord b)
            : r_{ clampCoord(r) }
            , g_{ clampCoord(g) }
            , b_{ clampCoord(b) }
        {}

        [[nodiscard]]
        friend cColor auto operator*(std::floating_point auto lhs, Color const& rhs) {
            using ResCoord = decltype(lhs * rhs.r_);
            return Color<ResCoord>{
                lhs * rhs.r_,
                lhs * rhs.g_,
                lhs * rhs.b_,
            };
        }

        [[nodiscard]]
        cColor auto operator+(Color const& other) const {
            using ResCoord = decltype(r_ + other.r());
            return Color<ResCoord>{
                r_ + other.r(),
                g_ + other.g(),
                b_ + other.b(),
            };
        }

        [[nodiscard]]
        Coord b() const {
            return b_;
        }

        [[nodiscard]]
        Coord g() const {
            return g_;
        }

        [[nodiscard]]
        Coord r() const {
            return r_;
        }

        [[nodiscard]]
        std::string svgCode() const {
            return std::format("#{:02X}{:02X}{:02X}", asByte(r_), asByte(g_), asByte(b_));
        }
    private:
        [[nodiscard]]
        static Coord clampCoord(Coord value) {
            return std::clamp<Coord>(value, 0.f, 1.f);
        }

        [[nodiscard]]
        static std::uint8_t asByte(Coord coord) {
            return std::uint8_t(std::floor(coord * 255.f + 0.5f));
        }

        Coord r_;
        Coord g_;
        Coord b_;
    };
}

template<gustave::cfg::cRealRep Coord>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::Color<Coord>> {
    using Color = gustave::examples::jsonGustave::Color<Coord>;

    [[nodiscard]]
    static Color from_json(nlohmann::json const& json) {
        auto const r = json.at("r").get<Coord>();
        auto const g = json.at("g").get<Coord>();
        auto const b = json.at("b").get<Coord>();
        return Color{ r, g, b };
    }
};
