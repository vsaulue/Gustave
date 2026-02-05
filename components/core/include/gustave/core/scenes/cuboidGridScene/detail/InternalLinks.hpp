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

#include <optional>

#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/utils/InplaceVector.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_>
    class InternalLinks {
    public:
        using SceneData = detail::SceneData<cfg, UD_>;

        using BlockData = SceneData::BlockData;
        using Direction = math3d::BasicDirection;
        using Value = DataNeighbour<cfg, UD_, false>;

        using Values = utils::InplaceVector<Value, 3>;
        using Iterator = Values::ConstIterator;

        [[nodiscard]]
        explicit InternalLinks(utils::NoInit)
            : source_{ nullptr }
        {}

        [[nodiscard]]
        explicit InternalLinks(SceneData const& scene, BlockIndex const& blockIndex)
            : source_{ &scene.blocks.at(blockIndex) }
        {
            auto processNeighbour = [&](Direction direction) {
                if (auto neighbourId = blockIndex.neighbourAlong(direction)) {
                    if (auto neighbourPtr = scene.blocks.find(*neighbourId)) {
                        if (!source_->isFoundation() || !neighbourPtr->isFoundation()) {
                            values_.emplaceBack(direction, *neighbourPtr);
                        }
                    }
                }
            };
            processNeighbour(Direction::plusX());
            processNeighbour(Direction::plusY());
            processNeighbour(Direction::plusZ());
        }

        [[nodiscard]]
        Value const& operator[](std::size_t index) const {
            return values_[index];
        }

        [[nodiscard]]
        Iterator begin() const {
            return values_.begin();
        }

        [[nodiscard]]
        Iterator end() const {
            return values_.end();
        }

        [[nodiscard]]
        std::size_t size() const {
            return values_.size();
        }

        [[nodiscard]]
        BlockData const& source() const {
            return *source_;
        }
    private:
        BlockData const* source_;
        Values values_;
    };
}
