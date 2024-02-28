# This file is part of Gustave, a structural integrity library for video games.
# 
# Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
# 
# MIT License
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

add_library(Gustave-Vanilla INTERFACE)
target_include_directories(Gustave-Vanilla INTERFACE "include")

target_link_libraries(Gustave-Vanilla
    INTERFACE Comp-Core
    INTERFACE Comp-Math
    INTERFACE Comp-Units-Vanilla
)

add_executable(Test-Gustave-Vanilla
    "tests/force1/solutionUtils/ContactReference.cpp"
    "tests/force1/solutionUtils/Contacts.cpp"
    "tests/force1/solutionUtils/NodeReference.cpp"
    "tests/force1/solutionUtils/Nodes.cpp"
    "tests/force1/Solution.cpp"
    "tests/force1/Solver.cpp"
    "tests/model/MaxStress.cpp"
    "tests/scenes/cuboidGrid/BlockConstructionInfo.cpp"
    "tests/scenes/cuboidGrid/BlockIndex.cpp"
    "tests/scenes/cuboidGrid/BlockReference.cpp"
    "tests/scenes/cuboidGrid/Blocks.cpp"
    "tests/scenes/cuboidGrid/ContactReference.cpp"
    "tests/scenes/cuboidGrid/Contacts.cpp"
    "tests/scenes/cuboidGrid/detail/BlockDataReference.cpp"
    "tests/scenes/cuboidGrid/detail/BlockMappedData.cpp"
    "tests/scenes/cuboidGrid/detail/DataNeighbours.cpp"
    "tests/scenes/cuboidGrid/detail/IndexNeighbours.cpp"
    "tests/scenes/cuboidGrid/detail/SceneBlocks.cpp"
    "tests/scenes/cuboidGrid/detail/SceneUpdater.cpp"
    "tests/scenes/cuboidGrid/StructureReference.cpp"
    "tests/scenes/cuboidGrid/Structures.cpp"
    "tests/scenes/cuboidGrid/Transaction.cpp"
    "tests/scenes/CuboidGridScene.cpp"
    "tests/worlds/sync/BlockReference.cpp"
    "tests/worlds/sync/Blocks.cpp"
    "tests/worlds/sync/detail/WorldUpdater.cpp"
    "tests/worlds/sync/StructureReference.cpp"
    "tests/worlds/sync/Structures.cpp"
    "tests/worlds/SyncWorld.cpp"
)
target_link_libraries(Test-Gustave-Vanilla
    PRIVATE Catch2::Catch2WithMain
    PRIVATE Comp-Testing
    PRIVATE Gustave-Vanilla
)
target_include_directories(Test-Gustave-Vanilla
    PRIVATE "tests/include"
)
declare_unit_test(Test-Gustave-Vanilla)
