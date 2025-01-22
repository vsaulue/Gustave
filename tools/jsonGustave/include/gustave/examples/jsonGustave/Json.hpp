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

#include <nlohmann/json.hpp>

#include <gustave/cfg/cReal.hpp>
#include <gustave/cfg/cVector3.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/cGustave.hpp>

namespace gustave::examples::jsonGustave {
    using Json = nlohmann::json;
}

template<typename Real>
    requires gustave::cfg::cReal<Real>
struct nlohmann::adl_serializer<Real> {
    using Rep = typename Real::Rep;
    static constexpr auto unit = Real::unit();

    [[nodiscard]]
    static Real from_json(nlohmann::json const& json) {
        return json.get<Rep>() * unit;
    }
};

template<typename Vector3>
    requires gustave::cfg::cVector3<Vector3>
struct nlohmann::adl_serializer<Vector3> {
    using Coord = typename Vector3::Coord;

    [[nodiscard]]
    static Vector3 from_json(nlohmann::json const& json) {
        auto const x = json.at("x").get<Coord>();
        auto const y = json.at("y").get<Coord>();
        auto const z = json.at("z").get<Coord>();
        return Vector3{ x, y, z };
    }
};

template<>
struct nlohmann::adl_serializer<gustave::core::scenes::cuboidGridScene::BlockIndex> {
    using BlockIndex = gustave::core::scenes::cuboidGridScene::BlockIndex;
    using Coord = BlockIndex::Coord;

    [[nodiscard]]
    static BlockIndex from_json(nlohmann::json const& json) {
        auto const x = json.at("x").get<Coord>();
        auto const y = json.at("y").get<Coord>();
        auto const z = json.at("z").get<Coord>();
        return BlockIndex{ x,y,z };
    }
};

template<typename Stress>
    requires gustave::core::model::cStress<Stress>
struct nlohmann::adl_serializer<Stress> {
public:
    using Coord = typename Stress::Coord;

    [[nodiscard]]
    static Stress from_json(nlohmann::json const& json) {
        auto const compression = getCoord(json, "compression");
        auto const shear = getCoord(json, "shear");
        auto const tensile = getCoord(json, "tensile");
        return Stress{ compression, shear, tensile };
    }
private:
    [[nodiscard]]
    static Coord getCoord(nlohmann::json const& json, char const* fieldName) {
        auto const result = json.at(fieldName).get<Coord>();
        if (result < Coord::zero()) {
            std::stringstream msg;
            msg << "Invalid Stress: field '" << fieldName << "' must be positive (passed: " << result << ").";
            throw std::invalid_argument(msg.str());
        }
        return result;
    }
};

