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
#include <unordered_set>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneBlocks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/IndexGenerator.hpp>
#include <gustave/utils/PointerHash.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto cfg, common::cSceneUserData UserData_>
    struct SceneData {
    public:
        using UserData = UserData_;

        using StructureData = detail::StructureData<cfg, UserData_>;
        using StructureIndex = cfg::StructureIndex<cfg>;
    private:
        static constexpr auto u = cfg::units(cfg);

        template<cfg::cUnitOf<cfg> auto unit>
        using Vector3 = cfg::Vector3<cfg, unit>;

        struct StructuresHelper {
        public:
            [[nodiscard]]
            static StructureIndex getIndex(StructureIndex v) {
                return v;
            }

            [[nodiscard]]
            static StructureIndex getIndex(std::shared_ptr<StructureData> const& data) {
                return data->index();
            }

            [[nodiscard]]
            static StructureIndex getIndex(std::shared_ptr<StructureData const> const& data) {
                return data->index();
            }

            [[nodiscard]]
            static StructureIndex getIndex(StructureData const* data) {
                return data->index();
            }

            struct Equals {
                using is_transparent = void;

                [[nodiscard]]
                bool operator()(auto const& lhs, auto const& rhs) const {
                    return getIndex(lhs) == getIndex(rhs);
                }
            };

            struct Hash {
                using is_transparent = void;

                [[nodiscard]]
                std::size_t operator()(auto const& value) const {
                    return std::hash<StructureIndex>{}(getIndex(value));
                }
            };

            using Set = std::unordered_set<std::shared_ptr<StructureData>, Hash, Equals>;
        };
    public:
        using Blocks = SceneBlocks<cfg>;
        using StructureIdGenerator = utils::IndexGenerator<StructureIndex>;
        using Structures = StructuresHelper::Set;

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

        [[nodiscard]]
        bool isStructureIdValid(StructureIndex id) const {
            return id != structureIdGenerator.invalidIndex() && structures.contains(id);
        }

        Blocks blocks;
        Structures structures;
        StructureIdGenerator structureIdGenerator;
    private:
        void resetSceneDataPtr() {
            for (auto& structure : structures) {
                structure->setSceneData(*this);
            }
        }
    };
}
