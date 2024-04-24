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
install(DIRECTORY "include/" COMPONENT Std-Gustave DESTINATION include)

target_link_libraries(Gustave-Vanilla
    INTERFACE Comp-Core
    INTERFACE Comp-Math
    INTERFACE Comp-Std-Strict-Unit
)

add_executable(Test-Gustave-Vanilla
    "tests/solvers/force1Solver/solution/ContactReference.cpp"
    "tests/solvers/force1Solver/solution/Contacts.cpp"
    "tests/solvers/force1Solver/solution/NodeReference.cpp"
    "tests/solvers/force1Solver/solution/Nodes.cpp"
    "tests/solvers/force1Solver/Config.cpp"
    "tests/solvers/force1Solver/Solution.cpp"
    "tests/solvers/Force1Solver.cpp"
    "tests/model/Stress.cpp"
    "tests/scenes/cuboidGridScene/BlockConstructionInfo.cpp"
    "tests/scenes/cuboidGridScene/BlockIndex.cpp"
    "tests/scenes/cuboidGridScene/BlockReference.cpp"
    "tests/scenes/cuboidGridScene/Blocks.cpp"
    "tests/scenes/cuboidGridScene/ContactReference.cpp"
    "tests/scenes/cuboidGridScene/Contacts.cpp"
    "tests/scenes/cuboidGridScene/detail/BlockDataReference.cpp"
    "tests/scenes/cuboidGridScene/detail/BlockMappedData.cpp"
    "tests/scenes/cuboidGridScene/detail/DataNeighbours.cpp"
    "tests/scenes/cuboidGridScene/detail/IndexNeighbours.cpp"
    "tests/scenes/cuboidGridScene/detail/InternalLinks.cpp"
    "tests/scenes/cuboidGridScene/detail/SceneBlocks.cpp"
    "tests/scenes/cuboidGridScene/detail/SceneUpdater.cpp"
    "tests/scenes/cuboidGridScene/Links.cpp"
    "tests/scenes/cuboidGridScene/StructureReference.cpp"
    "tests/scenes/cuboidGridScene/Structures.cpp"
    "tests/scenes/cuboidGridScene/Transaction.cpp"
    "tests/scenes/CuboidGridScene.cpp"
    "tests/worlds/syncWorld/BlockReference.cpp"
    "tests/worlds/syncWorld/Blocks.cpp"
    "tests/worlds/syncWorld/ContactReference.cpp"
    "tests/worlds/syncWorld/Contacts.cpp"
    "tests/worlds/syncWorld/detail/WorldUpdater.cpp"
    "tests/worlds/syncWorld/Links.cpp"
    "tests/worlds/syncWorld/StructureReference.cpp"
    "tests/worlds/syncWorld/Structures.cpp"
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
