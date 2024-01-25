/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <unordered_map>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/scenes/CuboidGridScene.hpp>
#include <gustave/solvers/Force1Solver.hpp>
#include <gustave/worlds/sync/detail/StructureData.hpp>

namespace Gustave::Worlds::Sync::detail {
    template<Cfg::cLibConfig auto cfg>
    struct WorldData {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;
    public:
        using Scene = Scenes::CuboidGridScene<cfg>;
        using Solver = Solvers::Force1Solver<cfg>;
        using StructureData = detail::StructureData<cfg>;

        using SceneStructure = typename Scene::StructureReference;
        using Structures = std::unordered_map<SceneStructure, std::shared_ptr<StructureData>>;

        [[nodiscard]]
        explicit WorldData(Vector3<u.length> const& blockSize, Solver solver_)
            : scene{ blockSize }
            , solver{ std::move(solver_) }
        {}

        Scene scene;
        Solver solver;
        Structures structures;
    };
}
