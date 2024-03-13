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

#include <array>
#include <optional>

#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockDataReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/BlockIndex.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid::detail {
    template<Cfg::cLibConfig auto cfg>
    class InternalLinks {
    public:
        using ConstBlockDataReference = detail::BlockDataReference<cfg, false>;
        using Direction = Math3d::BasicDirection;
        using SceneData = detail::SceneData<cfg>;

        struct Value {
            [[nodiscard]]
            Value()
                : otherBlock{ nullptr }
                , direction{ Direction::plusX() }
            {}

            [[nodiscard]]
            explicit Value(ConstBlockDataReference otherBlock_, Direction direction_)
                : otherBlock{ otherBlock_ }
                , direction{ direction_ }
            {}

            [[nodiscard]]
            bool operator==(Value const&) const = default;

            ConstBlockDataReference otherBlock;
            Direction direction;
        };

        using Values = std::array<Value, 3>;
        using Iterator = typename Values::const_iterator;

        [[nodiscard]]
        explicit InternalLinks(Utils::NoInit)
            : source_{ nullptr }
            , values_{}
            , size_{ 0 }
        {}

        [[nodiscard]]
        explicit InternalLinks(SceneData const& scene, BlockIndex const& blockIndex)
            : source_{ scene.blocks.find(blockIndex) }
            , values_{}
            , size_{ 0 }
        {
            assert(source_);
            auto processNeighbour = [&](Direction direction) {
                std::optional<BlockIndex> neighbourId = blockIndex.neighbourAlong(direction);
                if (neighbourId) {
                    ConstBlockDataReference neighbour = scene.blocks.find(*neighbourId);
                    if (neighbour) {
                        if (!source_.isFoundation() || !neighbour.isFoundation()) {
                            addValue(neighbour, direction);
                        }
                    }
                }
            };
            processNeighbour(Direction::plusX());
            processNeighbour(Direction::plusY());
            processNeighbour(Direction::plusZ());
        }

        [[nodiscard]]
        Value operator[](std::size_t index) const {
            return values_[index];
        }

        [[nodiscard]]
        Iterator begin() const {
            return values_.begin();
        }

        [[nodiscard]]
        Iterator end() const {
            return values_.begin() + size_;
        }

        [[nodiscard]]
        std::size_t size() const {
            return size_;
        }

        [[nodiscard]]
        ConstBlockDataReference source() const {
            return source_;
        }
    private:
        void addValue(ConstBlockDataReference neighbour, Direction direction) {
            values_[size_] = Value{ neighbour, direction };
            ++size_;
        }

        ConstBlockDataReference source_;
        Values values_;
        std::size_t size_;
    };
}
