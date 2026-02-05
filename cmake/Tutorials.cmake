# This file is part of Gustave, a structural integrity library for video games.
#
# Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

include_guard(GLOBAL)

include("cmake/CompilerOptions.cmake")
include("cmake/MemcheckTests.cmake")

set(tutorials "" CACHE INTERNAL "List of tutorial targets")

if(GUSTAVE_BUILD_TUTORIALS)
    add_custom_target(run-tutorials)

    find_package(CLI11 CONFIG REQUIRED)

    add_library(Tutorial INTERFACE)
    target_include_directories(Tutorial
        INTERFACE "${CMAKE_SOURCE_DIR}/docs/tutorials/include"
    )
    target_link_libraries(Tutorial
        INTERFACE CLI11::CLI11
    )
endif()

function(add_tutorial_executable)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET" "SOURCE_FILES;INCLUDE_DIRECTORIES")
    if("TARGET" IN_LIST ARG_KEYWORDS_MISSING_VALUES OR NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Missing mandatory TARGET argument.")
    endif()
    if(("SOURCE_FILES" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_SOURCE_FILES)
        message(FATAL_ERROR "Missing mandatory SOURCE_FILES argument.")
    endif()

    if(GUSTAVE_BUILD_TUTORIALS)
        add_executable("${ARG_TARGET}"
            ${ARG_SOURCE_FILES}
        )
        target_include_directories("${ARG_TARGET}"
            PRIVATE ${ARG_INCLUDE_DIRECTORIES}
        )
        target_link_libraries("${ARG_TARGET}"
            PRIVATE Gustave::Distrib-Std
            PRIVATE Tutorial
        )
        add_custom_target("run-${ARG_TARGET}"
            COMMAND ${ARG_TARGET}
            DEPENDS ${ARG_TARGET}
        )
        add_dependencies(run-tutorials "run-${ARG_TARGET}")
        list(APPEND tutorials "${ARG_TARGET}")
        set(tutorials "${tutorials}" CACHE INTERNAL "List of tutorial targets")
        add_test(NAME "${ARG_TARGET}"
            COMMAND "${ARG_TARGET}"
        )
        set_property(TEST "${ARG_TARGET}"
            PROPERTY LABELS "tutorial"
        )
        add_memcheck_test(
            TEST_ID "${ARG_TARGET}"
            TARGET "${ARG_TARGET}"
        )
    endif()
endfunction()
