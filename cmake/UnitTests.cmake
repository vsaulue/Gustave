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

include("cmake/CompilerOptions.cmake")
include("cmake/MemcheckTests.cmake")

set(unit_tests_enabled "${BUILD_TESTING}")

if(unit_tests_enabled)
    add_custom_target(run-unit-tests)

    set(unit_test_args)
    if(CMAKE_COLOR_DIAGNOSTICS)
        list(APPEND unit_test_args "--colour-mode" "ansi")
    endif()
endif()

function(add_unit_test)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET" "SOURCE_FILES;INCLUDE_DIRECTORIES;LINK_LIBRARIES")
    if("TARGET" IN_LIST ARG_KEYWORDS_MISSING_VALUES OR NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Missing mandatory TARGET argument.")
    endif()
    if(("SOURCE_FILES" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_SOURCE_FILES)
        message(FATAL_ERROR "Missing mandatory SOURCE_FILES argument.")
    endif()
    if(("LINK_LIBRARIES" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_LINK_LIBRARIES)
        message(FATAL_ERROR "Missing mandatory LINK_LIBRARIES argument.")
    endif()

    if(unit_tests_enabled)
        add_executable("${ARG_TARGET}"
            ${ARG_SOURCE_FILES}
        )
        target_include_directories("${ARG_TARGET}"
            PRIVATE ${ARG_INCLUDE_DIRECTORIES}
        )
        target_link_libraries("${ARG_TARGET}"
            PRIVATE ${ARG_LINK_LIBRARIES}
            PRIVATE Catch2::Catch2WithMain
        )
        add_custom_target("run-${ARG_TARGET}"
            COMMAND ${ARG_TARGET} ${unit_test_args}
            DEPENDS ${ARG_TARGET}
        )
        add_dependencies(run-unit-tests "run-${ARG_TARGET}")
        add_test(NAME "${ARG_TARGET}"
            COMMAND "${ARG_TARGET}" ${unit_test_args}
        )
        set_property(TEST "${ARG_TARGET}"
            PROPERTY LABELS "unit-test"
        )
        add_memcheck_test(
            TEST_ID "${ARG_TARGET}"
            TARGET "${ARG_TARGET}"
        )
    endif()
endfunction()
