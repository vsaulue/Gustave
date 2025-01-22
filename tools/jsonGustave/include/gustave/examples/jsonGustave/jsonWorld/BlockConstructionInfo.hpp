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
#include <gustave/examples/jsonGustave/Json.hpp>

namespace gustave::examples::jsonGustave::jsonWorld {
    template<core::cGustave G>
    class BlockConstructionInfo {
    public:
        using BlockIndex = typename G::Worlds::SyncWorld::BlockIndex;

        [[nodiscard]]
        explicit BlockConstructionInfo(BlockIndex const& index, std::string blockTypeName, bool isFoundation)
            : blockTypeName_{ std::move(blockTypeName) }
            , index_{ index }
            , isFoundation_{ isFoundation }
        {}

        [[nodiscard]]
        std::string const& blockTypeName() const {
            return blockTypeName_;
        }

        [[nodiscard]]
        BlockIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return isFoundation_;
        }
    private:
        std::string blockTypeName_;
        BlockIndex index_;
        bool isFoundation_;
    };
}

template<gustave::core::cGustave G>
struct nlohmann::adl_serializer<gustave::examples::jsonGustave::jsonWorld::BlockConstructionInfo<G>> {
    using BlockConstructionInfo = gustave::examples::jsonGustave::jsonWorld::BlockConstructionInfo<G>;

    using BlockIndex = typename BlockConstructionInfo::BlockIndex;

    [[nodiscard]]
    static BlockConstructionInfo from_json(nlohmann::json const& json) {
        auto index = json.at("index").get<BlockIndex>();
        auto blockTypeName = json.at("blockTypeName").get<std::string>();
        bool isFoundation = json.at("isFoundation").get<bool>();
        return BlockConstructionInfo{ index, std::move(blockTypeName), isFoundation };
    }
};
