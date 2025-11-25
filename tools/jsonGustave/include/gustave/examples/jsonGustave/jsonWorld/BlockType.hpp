/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <string>
#include <utility>

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/Color.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::jsonWorld {
    template<core::cGustave G>
    class BlockType {
    private:
        template<auto unit>
        using Real = G::template Real<unit>;

        static constexpr auto u = G::units();
    public:
        using Color = jsonGustave::Color<typename G::RealRep>;
        using PressureStress = G::Model::PressureStress;

        [[nodiscard]]
        explicit BlockType(std::string name, Color const& color, Real<u.mass> mass, PressureStress const& maxStress)
            : name_{ std::move(name) }
            , color_{ color }
            , mass_{ mass }
            , maxStress_{ maxStress }
        {}

        [[nodiscard]]
        Color const& color() const {
            return color_;
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return mass_;
        }

        [[nodiscard]]
        PressureStress const& maxStress() const {
            return maxStress_;
        }

        [[nodiscard]]
        std::string const& name() const {
            return name_;
        }
    private:
        std::string name_;
        Color color_;
        Real<u.mass> mass_;
        PressureStress maxStress_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::jsonWorld::BlockType<G>> {
    using Float = G::RealRep;

    static constexpr auto u = G::units();

    template<auto unit>
    using Real = G::template Real<unit>;

    using PressureStress = G::Model::PressureStress;
    using BlockType = gustave::examples::jsonGustave::jsonWorld::BlockType<G>;
    using Color = gustave::examples::jsonGustave::Color<Float>;

    [[nodiscard]]
    static BlockType from_json(nlohmann::json const& json) {
        static_assert(gustave::core::model::cStress<PressureStress>);
        auto name = json.at("name").get<std::string>();
        auto color = json.at("color").get<Color>();
        auto mass = json.at("mass").get<Real<u.mass>>();
        auto maxStress = json.at("maxStress").get<PressureStress>();
        return BlockType{ std::move(name), color, mass, maxStress };
    }
};
