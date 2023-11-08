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
#include <utility>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/scenes/cuboidGrid/Scene.hpp>
#include <gustave/solvers/force1/Solver.hpp>
#include <gustave/utils/PointerHash.hpp>
#include <gustave/world/WorldStructureState.hpp>

namespace Gustave::World {
    template<Cfg::cLibConfig auto cfg>
    class SyncWorld {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using Scene = Scenes::CuboidGrid::Scene<cfg>;
        using Solver = Solvers::Force1::Solver<cfg>;
        class WorldStructure;

        using BlockIndex = typename Scene::BlockIndex;
        using SceneStructure = typename Scene::StructureData;
        using SceneTransactionResult = typename Scene::TransactionResult;
        using Solution = typename Solver::Solution;
        using SolverStructure = typename Solver::Structure;
        using SolverProblem = typename Solver::Problem;
        using Structures = Utils::PointerHash::Map<SceneStructure const*, std::shared_ptr<WorldStructure>>;
        using Transaction = typename Scene::Transaction;

        class WorldStructure {
        public:
            using State = WorldStructureState;

            [[nodiscard]]
            explicit WorldStructure(std::shared_ptr<SceneStructure const> sceneStructure)
                : sceneStructure_{ std::move(sceneStructure) }
                , state_{ WorldStructureState::New }
            {
                assert(sceneStructure_);
            }

            [[nodiscard]]
            bool contains(BlockIndex const& blockIndex) const {
                return sceneStructure_->contains(blockIndex);
            }

            [[nodiscard]]
            std::optional<Vector3<u.force>> forceVector(BlockIndex const& to, BlockIndex const& from) const {
                assert(state_ == State::Solved);
                auto const toIndex = sceneStructure_->solverIndexOf(to);
                auto const fromIndex = sceneStructure_->solverIndexOf(from);
                if (toIndex && fromIndex) {
                    return solution_->forceVector(*toIndex, *fromIndex);
                } else {
                    return {};
                }
            }

            void invalidate() {
                state_ = State::Invalid;
            }

            [[nodiscard]]
            SceneStructure const& sceneStructure() const {
                return *sceneStructure_.get();
            }

            [[nodiscard]]
            std::shared_ptr<SceneStructure const> const& sceneStructurePtr() const {
                return sceneStructure_;
            }

            [[nodiscard]]
            std::shared_ptr<SolverStructure const> solverStructurePtr() const {
                return sceneStructure_->solverStructurePtr();
            }

            void solve(std::shared_ptr<Solution const> solution) {
                assert(state_ == State::New);
                assert(solution);
                solution_ = std::move(solution);
                state_ = State::Solved;
            }

            [[nodiscard]]
            State state() const {
                return state_;
            }
        private:
            std::shared_ptr<SceneStructure const> sceneStructure_;
            std::shared_ptr<Solution const> solution_;
            State state_;
        };

        [[nodiscard]]
        explicit SyncWorld(Vector3<u.length> const& blockSize, Vector3<u.acceleration> const& g, Solver solver)
            : g_{ g }
            , scene_{ blockSize }
            , solver_{ std::move(solver) }
        {}

        [[nodiscard]]
        std::shared_ptr<WorldStructure const> anyStructureContaining(BlockIndex const& blockIndex) const {
            auto const structures = scene_.blocks().at(blockIndex).structures();
            auto const it = structures.begin();
            if (it != structures.end()) {
                return structures_.at(*it);
            } else {
                return { nullptr };
            }
        }

        void modify(Transaction const& transaction) {
            SceneTransactionResult const trResult = scene_.modify(transaction);
            for (SceneStructure const* structure : trResult.deletedStructures()) {
                auto const count = structures_.erase(structure);
                assert(count == 1);
            }
            for (std::shared_ptr<SceneStructure const> const& sceneStructure : trResult.newStructures()) {
                std::shared_ptr<WorldStructure> worldStructure = std::make_shared<WorldStructure>(sceneStructure);
                auto const result = solver_.run(SolverProblem{ g_, worldStructure->solverStructurePtr() });
                worldStructure->solve(result.solutionPtr());
                auto const insertResult = structures_.insert({ &worldStructure->sceneStructure(), std::move(worldStructure)});
                assert(insertResult.second);
            }
        }
    private:
        Vector3<u.acceleration> g_;
        Scene scene_;
        Solver solver_;
        Structures structures_;
    };
}
