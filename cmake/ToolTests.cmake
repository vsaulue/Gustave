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

set(tool_tests_enabled "${BUILD_TESTING}")

if(tool_tests_enabled)
    add_custom_target(run-tool-tests)
endif()

function(add_tool_test)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TEST_ID" "COMMAND;DEPENDS")
    if (("COMMAND" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_COMMAND)
        message(FATAL_ERROR "Missing mandatory COMMAND argument.")
    endif()
    if (("DEPENDS" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_DEPENDS)
        message(FATAL_ERROR "Missing mandatory DEPENDS argument (should at least contain the tested tool).")
    endif()
    if (("TEST_ID" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_TEST_ID)
        message(FATAL_ERROR "Missing mandatory TEST_ID argument.")
    endif()

    if(tool_tests_enabled)
        add_custom_target("run-${ARG_TEST_ID}"
            COMMAND ${ARG_COMMAND}
            DEPENDS ${ARG_DEPENDS}
        )
        add_dependencies(run-tool-tests "run-${ARG_TEST_ID}")

        add_test(NAME "${ARG_TEST_ID}"
            COMMAND ${ARG_COMMAND}
        )
        set_property(TEST "${ARG_TEST_ID}"
            PROPERTY LABELS "tool-test"
        )
    endif()
endfunction()
