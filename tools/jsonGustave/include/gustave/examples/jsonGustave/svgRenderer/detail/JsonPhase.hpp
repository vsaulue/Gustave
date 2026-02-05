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

#include <gustave/core/cGustave.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/BlockTypePhase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/BlockStressPhase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/ContactStressPhase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/Phase.hpp>
#include <gustave/examples/jsonGustave/svgRenderer/phases/WorldFramePhase.hpp>
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::svgRenderer::detail {
    template<core::cGustave G>
    struct JsonPhase {
        using Phase = phases::Phase<G>;

        std::unique_ptr<Phase> ptr;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::svgRenderer::detail::JsonPhase<G>> {
    using JsonPhase = gustave::examples::jsonGustave::svgRenderer::detail::JsonPhase<G>;

    using BlockStressPhase = gustave::examples::jsonGustave::svgRenderer::phases::BlockStressPhase<G>;
    using BlockTypePhase = gustave::examples::jsonGustave::svgRenderer::phases::BlockTypePhase<G>;
    using ContactStressPhase = gustave::examples::jsonGustave::svgRenderer::phases::ContactStressPhase<G>;
    using WorldFramePhase = gustave::examples::jsonGustave::svgRenderer::phases::WorldFramePhase<G>;

    using PhaseMaker = JsonPhase(*)(nlohmann::json const& json);

    [[nodiscard]]
    static JsonPhase from_json(nlohmann::json const& json) {
        static std::unordered_map<std::string, PhaseMaker> const nameToFactory = initNameToFactory();
        auto const type = json.at("type").get<std::string>();
        auto itFactory = nameToFactory.find(type);
        if (itFactory == nameToFactory.end()) {
            std::stringstream msg;
            msg << "Unknown SvgRenderer Phase 'type': '" << type << "'.";
            throw std::invalid_argument(msg.str());
        }
        return itFactory->second(json);
    }
private:
    template<typename Phase>
    [[nodiscard]]
    static JsonPhase makePhase(nlohmann::json const& json) {
        return { std::make_unique<Phase>(json.get<Phase>()) };
    }

    [[nodiscard]]
    static std::unordered_map<std::string, PhaseMaker> initNameToFactory() {
        std::unordered_map<std::string, PhaseMaker> result;
        result["blockType"] = makePhase<BlockTypePhase>;
        result["blockStress"] = makePhase<BlockStressPhase>;
        result["contactStress"] = makePhase<ContactStressPhase>;
        result["worldFrame"] = makePhase<WorldFramePhase>;
        return result;
    }
};
