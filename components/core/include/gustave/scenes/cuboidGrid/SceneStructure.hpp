/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <cassert>
#include <optional>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/model/SolverStructure.hpp>
#include <gustave/scenes/cuboidGrid/detail/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class SceneStructure {
    private:
        static constexpr auto u = Cfg::units(cfg);

        using ConstBlockReference = detail::BlockReference<cfg, false>;
        using Material = Model::Material<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using SceneBlocks = detail::SceneBlocks<cfg>;
        using SolverStructure = Model::SolverStructure<cfg>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;
    public:
        [[nodiscard]]
        SceneStructure(SceneBlocks const& sceneBlocks)
            : sceneBlocks_{ sceneBlocks }
        {

        }

        void addBlock(ConstBlockReference block) {
            assert(block);
            NodeIndex const newIndex = solverNodes().size();
            auto insertResult = solverIndices_.insert({ block, newIndex });
            if (insertResult.second) {
                solverNodes().emplace_back(block.mass(), block.isFoundation());
            }
        }

        void addContact(ConstBlockReference b1, ConstBlockReference b2, NormalizedVector3 const& normalOnB1,
                        Real<u.area> area, Real<u.length> thickness, Material const& maxConstraints)
        {
            solverStructure_.addLink({ indexOf(b1), indexOf(b2), normalOnB1, area, thickness, maxConstraints });
        }

        [[nodiscard]]
        bool contains(ConstBlockReference block) const {
            return solverIndices_.contains(block);
        }

        [[nodiscard]]
        bool contains(BlockPosition const& position) const {
            ConstBlockReference block = sceneBlocks_.find(position);
            if (block) {
                return contains(block);
            } else {
                return false;
            }
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(ConstBlockReference block) const {
            auto const findResult = solverIndices_.find(block);
            if (findResult != solverIndices_.end()) {
                return { findResult->second };
            } else {
                return {};
            }
        }

        [[nodiscard]]
        std::optional<NodeIndex> solverIndexOf(BlockPosition const& position) const {
            ConstBlockReference const block = sceneBlocks_.find(position);
            if (block) {
                return solverIndexOf(block);
            } else {
                return {};
            }
        }

        [[nodiscard]]
        SolverStructure const& solverStructure() const {
            return solverStructure_;
        }
    private:
        [[nodiscard]]
        std::vector<Model::SolverNode<cfg>>& solverNodes() {
            return solverStructure_.nodes();
        }

        [[nodiscard]]
        NodeIndex indexOf(ConstBlockReference block) const {
            return solverIndices_.at(block);
        }

        SceneBlocks const& sceneBlocks_;
        SolverStructure solverStructure_;
        std::unordered_map<ConstBlockReference, NodeIndex> solverIndices_;
    };
}
