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

include("cmake/PythonTests.cmake")

add_custom_target(run-memcheck-tests)

set(memcheck_type "")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(memcheck_type "drMemory")
    find_program(drMemory_exe "drmemory" HINTS "${GUSTAVE_MEMCHECKER_PATH}")
    message(STATUS "Gustave: using drMemory at: '${drMemory_exe}'.")
else()
    set(memcheck_type "valgrind")
    find_program(valgrind_exe "valgrind" HINTS "${GUSTAVE_MEMCHECKER_PATH}")
    message(STATUS "Gustave: using valgrind at: '${valgrind_exe}'.")
endif()

function(declare_memcheck_test)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TEST_ID;TARGET" "ARGS")
    if (ARG_TEST_ID IN_LIST ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Missing mandatory TEST_ID argument.")
    endif()
    if (ARG_TARGET IN_LIST ARG_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "Missing mandatory TARGET argument.")
    endif()
    get_target_property(target_type "${ARG_TARGET}" TYPE)
    if (NOT target_type STREQUAL "EXECUTABLE")
        message(FATAL_ERROR "TARGET expects an executable target (provied: '${target_type}').")
    endif()

    set(new_memcheck_target "run-memcheck-${ARG_TEST_ID}")
    if(TARGET "${new_memcheck_target}")
        message(FATAL_ERROR "TEST_ID '${ARG_TEST_ID}' is already used.")
    endif()

    add_custom_target("${new_memcheck_target}"
        COMMAND Python::Interpreter "${python_scripts_dir}/runMemcheckTest.py"
            "--cmakeVariables" "${cmake_variables_json}"
            "--"
            "$<TARGET_FILE:${ARG_TARGET}>"
            ${ARG_ARGS}
        DEPENDS "${ARG_TARGET}"
    )
    add_dependencies(run-memcheck-tests "${new_memcheck_target}")
endfunction()
