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
#include <gustave/core/scenes/CuboidGridScene.hpp>
#include <gustave/core/worlds/syncWorld/StructureState.hpp>

namespace gustave::core::worlds::syncWorld::detail {
    template<cfg::cLibConfig auto libCfg>
    struct WorldData;

    template<cfg::cLibConfig auto libCfg>
    class StructureData {
    public:
        using WorldData = detail::WorldData<libCfg>;
        using Scene = typename WorldData::Scene;
        using Solver = typename WorldData::Solver;
        using State = StructureState;

        using SceneStructure = typename Scene::StructureReference;
        using Solution = typename Solver::Solution;
        using SolverStructure = typename Solver::Structure;

        [[nodiscard]]
        explicit StructureData(WorldData const& world, SceneStructure sceneStructure)
            : world_{ &world }
            , sceneStructure_{ std::move(sceneStructure) }
            , solution_{ nullptr }
            , state_{ State::New }
        {
            assert(sceneStructure_.isValid());
        }

        void invalidate() {
            state_ = State::Invalid;
        }

        [[nodiscard]]
        SceneStructure const& sceneStructure() const {
            return sceneStructure_;
        }

        [[nodiscard]]
        Solution const& solution() const {
            if (state_ != State::Solved) {
                throw std::logic_error("The structure must be in the 'Solved' state.");
            }
            return *solution_;
        }

        void solve(std::shared_ptr<Solution const> solution) {
            assert(state_ == State::New);
            if (solution != nullptr) {
                assert(&solution->basis().structure() == &sceneStructure_.solverStructure());
                assert(&solution->basis().config() == &world_->solver.config());
                solution_ = std::move(solution);
                state_ = State::Solved;
            } else {
                state_ = State::Unsolvable;
            }
        }

        [[nodiscard]]
        State state() const {
            return state_;
        }

        void setWorldData(WorldData const& value) {
            world_ = &value;
        }

        [[nodiscard]]
        WorldData const& world() const {
            return *world_;
        }
    private:
        WorldData const* world_;
        SceneStructure sceneStructure_;
        std::shared_ptr<Solution const> solution_;
        State state_;
    };
}
