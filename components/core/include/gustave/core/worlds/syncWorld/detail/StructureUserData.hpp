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
#include <gustave/core/solvers/Force1Solver.hpp>
#include <gustave/core/worlds/syncWorld/StructureState.hpp>
#include <gustave/core/worlds/syncWorld/forwardDecls.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::worlds::syncWorld::detail {
    template<cfg::cLibConfig auto libCfg_>
    class StructureUserData {
    public:
        using Solver = solvers::Force1Solver<libCfg_>;
        using State = StructureState;
        using WorldData = detail::WorldData<libCfg_>;

        using Solution = Solver::Solution;
        using SolverStructure = Solver::Structure;

        [[nodiscard]]
        StructureUserData()
            : world_{ nullptr }
            , solution_{ nullptr }
            , state_{ State::New }
        {}

        void init(WorldData& world) {
            assert(state_ == State::New);
            assert(world_ == nullptr);
            world_ = &world;
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

        void setWorldData(WorldData& value) {
            world_ = &value;
        }

        [[nodiscard]]
        WorldData const& world() const {
            return *world_;
        }
    private:
        utils::prop::Ptr<WorldData> world_;
        std::shared_ptr<Solution const> solution_;
        State state_;
    };
}
