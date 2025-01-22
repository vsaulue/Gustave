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

#include <functional>
#include <optional>
#include <ostream>

#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/utils/Hasher.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    class ContactIndex {
    public:
        using BlockIndex = cuboidGridScene::BlockIndex;
        using Direction = math3d::BasicDirection;

        [[nodiscard]]
        explicit ContactIndex(utils::NoInit NO_INIT)
            : localBlockIndex_{ NO_INIT }
            , direction_{ Direction::plusX() }
        {}

        [[nodiscard]]
        explicit ContactIndex(BlockIndex const& localBlockIndex, Direction direction)
            : localBlockIndex_{ localBlockIndex }
            , direction_{ direction }
        {}

        [[nodiscard]]
        Direction direction() const {
            return direction_;
        }

        [[nodiscard]]
        BlockIndex const& localBlockIndex() const {
            return localBlockIndex_;
        }

        [[nodiscard]]
        std::optional<ContactIndex> opposite() const {
            std::optional<BlockIndex> otherId = otherBlockIndex();
            if (otherId) {
                return ContactIndex{ *otherId, direction_.opposite() };
            } else {
                return {};
            }
        }

        [[nodiscard]]
        std::optional<BlockIndex> otherBlockIndex() const {
            return localBlockIndex_.neighbourAlong(direction_);
        }

        [[nodiscard]]
        bool operator==(ContactIndex const&) const = default;

        [[nodiscard]]
        friend std::ostream& operator<<(std::ostream& stream, ContactIndex const& index) {
            return stream << "{ blockIndex: " << index.localBlockIndex_ << ", direction: " << index.direction_ << " }";
        }
    private:
        BlockIndex localBlockIndex_;
        Direction direction_;
    public:
        using Hasher = utils::Hasher<ContactIndex, &ContactIndex::localBlockIndex_, &ContactIndex::direction_>;
    };
}

template<>
struct std::hash<gustave::core::scenes::cuboidGridScene::ContactIndex>
    : public gustave::core::scenes::cuboidGridScene::ContactIndex::Hasher
{};
