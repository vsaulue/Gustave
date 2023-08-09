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

#include <gustave/cfg/LibTraits.hpp>
#include <gustave/model/MaxStress.hpp>
#include <gustave/scenes/cuboidGrid/BlockConstructionInfo.hpp>
#include <gustave/scenes/cuboidGrid/BlockPosition.hpp>
#include <gustave/scenes/cuboidGrid/Scene.hpp>
#include <gustave/scenes/cuboidGrid/SceneStructure.hpp>
#include <gustave/scenes/cuboidGrid/Transaction.hpp>
#include <gustave/solvers/force1/Solution.hpp>
#include <gustave/solvers/force1/SolutionBasis.hpp>
#include <gustave/solvers/force1/Solver.hpp>
#include <gustave/solvers/SolverContact.hpp>
#include <gustave/solvers/SolverNode.hpp>
#include <gustave/solvers/SolverStructure.hpp>
#include <gustave/units/lib/Unit.hpp>

#include "LibConfig.hpp"

namespace Gustave::Vanilla {
    inline constexpr LibConfig libConfig{};

    using MaxStress = Model::MaxStress<libConfig>;

    using SolverContact = Solvers::SolverContact<libConfig>;
    using SolverNode = Solvers::SolverNode<libConfig>;
    using SolverStructure = Solvers::SolverStructure<libConfig>;

    namespace Force1 {
        using Solution = Solvers::Force1::Solution<libConfig>;
        using SolutionBasis = Solvers::Force1::SolutionBasis<libConfig>;
        using Solver = Solvers::Force1::Solver<libConfig>;
    }

    namespace CuboidGrid {
        using BlockConstructionInfo = Scenes::CuboidGrid::BlockConstructionInfo<libConfig>;
        using BlockPosition = Scenes::CuboidGrid::BlockPosition;
        using Scene = Scenes::CuboidGrid::Scene<libConfig>;
        using SceneStructure = Scenes::CuboidGrid::Scene<libConfig>;
        using Transaction = Scenes::CuboidGrid::Transaction<libConfig>;
    }

    using NormalizedVector3 = LibConfig::NormalizedVector3;

    template<Units::Lib::cUnit auto unit>
    using Vector3 = LibConfig::Vector3<unit>;

    template<Units::Lib::cUnit auto unit>
    using Real = Cfg::Real<libConfig, unit>;

    using ContactIndex = LibConfig::ContactIndex;
    using NodeIndex = LibConfig::NodeIndex;
}
