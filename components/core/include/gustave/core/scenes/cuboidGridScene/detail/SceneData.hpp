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

#include <memory>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneBlocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/utils/IndexGenerator.hpp>
#include <gustave/utils/PointerHash.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg>
    class StructureData;

    template<cfg::cLibConfig auto cfg>
    struct SceneData {
    private:
        static constexpr auto u = cfg::units(cfg);

        template<cfg::cUnitOf<cfg> auto unit>
        using Vector3 = cfg::Vector3<cfg, unit>;
    public:
        using Blocks = SceneBlocks<cfg>;
        using StructureData = detail::StructureData<cfg>;
        using StructureIndex = cfg::StructureIndex<cfg>;
        using Structures = utils::PointerHash::Set<std::shared_ptr<StructureData>>;

        [[nodiscard]]
        explicit SceneData(Vector3<u.length> const& blockSize)
            : blocks{ blockSize }
        {}

        SceneData(SceneData const&) = delete;
        SceneData& operator=(SceneData const&) = delete;

        [[nodiscard]]
        SceneData(SceneData&& other)
            : blocks{ std::move(other.blocks) }
            , structures{ std::move(other.structures) }
        {
            resetSceneDataPtr();
        }

        SceneData& operator=(SceneData&& other) {
            if (&other != this) {
                blocks = std::move(other.blocks);
                structures = std::move(other.structures);
                resetSceneDataPtr();
            }
            return *this;
        }

        [[nodiscard]]
        bool isStructureValid(StructureData const* structure) const {
            return structure != nullptr && structures.contains(structure);
        }

        Blocks blocks;
        Structures structures;
        utils::IndexGenerator<StructureIndex> structureIdGenerator;
    private:
        void resetSceneDataPtr() {
            for (auto& structure : structures) {
                structure->setSceneData(*this);
            }
        }
    };
}
