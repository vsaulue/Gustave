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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_, bool isMut_>
    class DataNeighbour {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;
    public:
        using BlockData = detail::BlockData<cfg, UD_>;
        using Direction = math3d::BasicDirection;

        [[nodiscard]]
        explicit DataNeighbour(Direction direction, Prop<BlockData>& otherBlock)
            : direction_{ direction }
            , otherBlock_{ &otherBlock }
        {}

        [[nodiscard]]
        bool operator==(DataNeighbour const& other) const = default;

        [[nodiscard]]
        Direction direction() const {
            return direction_;
        }

        [[nodiscard]]
        Prop<BlockData>& otherBlock() const {
            return *otherBlock_;
        }
    private:
        Direction direction_;
        Prop<BlockData>* otherBlock_;
    };
}
