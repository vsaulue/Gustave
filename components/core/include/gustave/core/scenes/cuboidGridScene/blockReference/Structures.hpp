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

#include <array>
#include <cstddef>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::blockReference {
    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
    class Structures {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        using BlockDataRef = cuboidGridScene::detail::BlockDataReference<libCfg_, false>;
        using DataNeighbours = cuboidGridScene::detail::DataNeighbours<libCfg_, false>;
        using SceneData = cuboidGridScene::detail::SceneData<libCfg_, UD_>;
        using StructureReference = cuboidGridScene::StructureReference<libCfg_, UD_, isMut_>;
        using StructureIndex = StructureReference::StructureIndex;

        using Values = std::array<StructureReference, 6>;
    public:
        using ConstIterator = Values::const_iterator;
        using Iterator = Values::iterator;

        [[nodiscard]]
        explicit Structures(Prop<SceneData>& scene, BlockDataRef blockData)
            : sceneStructures_{ NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT(), NO_INIT() }
            , size_{ 0 }
        {
            auto addValue = [&](StructureIndex structId) {
                if (!contains(structId)) {
                    sceneStructures_[size_] = StructureReference{ scene.structures.atShared(structId) };
                    ++size_;
                }
            };
            if (blockData.isFoundation()) {
                for (auto const& neighbour : DataNeighbours{ scene.blocks, blockData.index() }) {
                    auto const nBlockData = neighbour.block;
                    if (!nBlockData.isFoundation()) {
                        addValue(nBlockData.structureId());
                    }
                }
            } else {
                addValue(blockData.structureId());
            }
        }

        [[nodiscard]]
        StructureReference& operator[](std::size_t index) {
            return sceneStructures_[index];
        }

        [[nodiscard]]
        StructureReference const& operator[](std::size_t index) const {
            return sceneStructures_[index];
        }

        [[nodiscard]]
        Iterator begin() {
            return sceneStructures_.begin();
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return sceneStructures_.begin();
        }

        [[nodiscard]]
        ConstIterator end() const {
            return begin() + size_;
        }

        [[nodiscard]]
        std::size_t size() const {
            return size_;
        }
    private:
        [[nodiscard]]
        static StructureReference NO_INIT() {
            return StructureReference{ utils::NO_INIT };
        }

        [[nodiscard]]
        bool contains(StructureIndex structId) const {
            for (std::size_t id = 0; id < size_; ++id) {
                if (structId == sceneStructures_[id].index()) {
                    return true;
                }
            }
            return false;
        }

        Values sceneStructures_;
        std::size_t size_;
    };
}
