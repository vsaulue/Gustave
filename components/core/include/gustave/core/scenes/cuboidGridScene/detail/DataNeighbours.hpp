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
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbour.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/IndexNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/InplaceVector.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UD_, bool isMut_>
    class DataNeighbours {
    public:
        using SceneData = detail::SceneData<cfg, UD_>;

        using Neighbour = DataNeighbour<cfg, UD_, isMut_>;
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        using Values = utils::InplaceVector<Neighbour, 6>;
    public:
        using Iterator = Values::Iterator;
        using EndIterator = Values::ConstIterator;

        [[nodiscard]]
        explicit DataNeighbours(Prop<SceneData>& scene, BlockIndex const& source) {
            for (auto const& indexNeighbour : IndexNeighbours{ source }) {
                if (auto neighbour = scene.blocks.find(indexNeighbour.index)) {
                    values_.emplaceBack(indexNeighbour.direction, *neighbour);
                }
            }
        }

        [[nodiscard]]
        Iterator begin() {
            return values_.begin();
        }

        [[nodiscard]]
        EndIterator end() const {
            return values_.end();
        }
    private:
        Values values_;
    };
}
