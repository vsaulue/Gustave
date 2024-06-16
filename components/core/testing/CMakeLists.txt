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

function(add_test_core test_target_name config_lib)
    add_executable("${test_target_name}"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/model/Stress.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/BlockConstructionInfo.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/BlockIndex.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/BlockReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/Blocks.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/ContactReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/Contacts.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/BlockDataReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/BlockMappedData.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/DataNeighbours.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/IndexNeighbours.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/InternalLinks.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/SceneBlocks.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/SceneData.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/detail/SceneUpdater.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/Links.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/StructureReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/Structures.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/cuboidGridScene/Transaction.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scenes/CuboidGridScene.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/detail/LayerStructure.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/solution/ContactReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/solution/Contacts.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/solution/NodeReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/solution/Nodes.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/Config.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/force1Solver/Solution.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/solvers/Force1Solver.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/detail/WorldUpdater.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/BlockReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/Blocks.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/ContactReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/Contacts.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/Links.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/StructureReference.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/syncWorld/Structures.cpp"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/worlds/SyncWorld.cpp"
    )
    target_link_libraries("${test_target_name}"
        PRIVATE Catch2::Catch2WithMain
        PRIVATE "${config_lib}"
        PRIVATE Comp-Core
        PRIVATE Comp-Testing
    )
    target_include_directories("${test_target_name}"
        PRIVATE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include"
    )
    declare_unit_test("${test_target_name}")
endfunction()
