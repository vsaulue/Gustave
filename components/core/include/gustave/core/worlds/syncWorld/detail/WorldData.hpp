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
#include <gustave/core/worlds/syncWorld/detail/StructureUserData.hpp>
#include <gustave/core/worlds/syncWorld/StructureState.hpp>

namespace gustave::core::worlds::syncWorld::detail {
    template<cfg::cLibConfig auto libCfg>
    struct WorldData {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        struct SceneUserData {
            using Structure = StructureUserData<libCfg>;
        };
    public:
        using Scene = scenes::CuboidGridScene<libCfg, SceneUserData>;
        using Solver = SceneUserData::Structure::Solver;
        using StructureState = syncWorld::StructureState;

        [[nodiscard]]
        explicit WorldData(Vector3<u.length> const& blockSize, Solver solver_)
            : scene{ blockSize }
            , solver{ std::move(solver_) }
        {}

        WorldData(WorldData const&) = delete;
        WorldData& operator=(WorldData const&) = delete;

        [[nodiscard]]
        WorldData(WorldData&& other)
            : scene{ std::move(other.scene) }
            , solver{ std::move(other.solver) }
        {
            resetWorldDataPtr();
        }

        WorldData& operator=(WorldData&& other) {
            if (&other != this) {
                scene = std::move(other.scene);
                solver = std::move(other.solver);
                resetWorldDataPtr();
            }
            return *this;
        }

        Scene scene;
        Solver solver;
    private:
        void resetWorldDataPtr() {
            for (auto& structure : scene.structures()) {
                structure.userData().setWorldData(*this);
            }
        }
    };
}
