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

include_guard(GLOBAL)

include("cmake/MemcheckTests.cmake")

add_custom_target(build-tests)
add_custom_target(run-tests)

set(test_compile_options)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    list(APPEND test_compile_options "/W3" "/WX" "/permissive-" "/utf-8")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    list(APPEND test_compile_options "-Wall" "-Wextra" "-Wpedantic" "-Werror" "-fconcepts-diagnostics-depth=5")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    list(APPEND test_compile_options "-Wall" "-Wextra" "-Wpedantic" "-Werror")
endif()

set(test_command_args)

if(CMAKE_COLOR_DIAGNOSTICS)
    list(APPEND test_command_args "--colour-mode" "ansi")
endif()

function(declare_unit_test test_target_name)
    add_dependencies(build-tests "${test_target_name}")
    add_custom_target("run-${test_target_name}"
        COMMAND ${test_target_name} ${test_command_args}
        DEPENDS ${test_target_name}
    )
    target_compile_options(${test_target_name}
        PUBLIC ${test_compile_options}
    )
    add_dependencies(run-tests "run-${test_target_name}")
    declare_memcheck_test(
        TEST_ID "unitTest-${test_target_name}"
        TARGET "${test_target_name}"
    )
endfunction()
