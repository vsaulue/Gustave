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

#include <string>

#include <gustave/cfg/cReal.hpp>
#include <gustave/core/model/Stress.hpp>

namespace gustave::examples::jsonGustave {
    class StressCoord {
    public:
        enum class Id { compression, shear, tensile, max };

        [[nodiscard]]
        StressCoord(Id id)
            : id_{ id }
        {
            if (static_cast<int>(id_) > 3) {
                throw invalidError();
            }
        }

        [[nodiscard]]
        cfg::cReal auto extract(core::model::cStress auto const& stress) const {
            switch (id_) {
            case Id::compression:
                return stress.compression();
            case Id::shear:
                return stress.shear();
            case Id::tensile:
                return stress.tensile();
            case Id::max:
                return stress.maxCoord();
            }
            throw invalidError();
        }

        [[nodiscard]]
        Id id() const {
            return id_;
        }

        [[nodiscard]]
        std::logic_error invalidError() const {
            std::string msg = "Invalid StressCoord : ";
            msg += static_cast<int>(id_);
            msg += '.';
            return std::logic_error{ msg };
        }
    private:
        Id id_;
    };
}

template<>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::StressCoord> {
public:
    using StressCoord = gustave::examples::jsonGustave::StressCoord;
    using Id = StressCoord::Id;

    [[nodiscard]]
    static StressCoord from_json(nlohmann::json const& json) {
        static auto const nameToValue = initNameToValue();
        auto const name = json.get<std::string>();
        auto it = nameToValue.find(name);
        if (it == nameToValue.end()) {
            std::stringstream msg;
            msg << "Unknown StressCoord: '" << name << "'.";
            throw std::invalid_argument(msg.str());
        }
        return it->second;
    }
private:
    [[nodiscard]]
    static std::unordered_map<std::string, StressCoord> initNameToValue() {
        std::unordered_map<std::string, StressCoord> result;
        result.insert({ "compression", Id::compression });
        result.insert({ "shear", Id::shear });
        result.insert({ "tensile", Id::tensile });
        result.insert({ "max", Id::max });
        return result;
    }
};

