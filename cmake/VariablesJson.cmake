# This file is part of Gustave, a structural integrity library for video games.
#
# Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
include("cmake/PackagingTests.cmake")

set(cmake_variables_json "${CMAKE_CURRENT_BINARY_DIR}/cmake-variables-$<CONFIG>.json")

function(json_string)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "VALUE;OUTPUT_VARIABLE" "")
    if (("OUTPUT_VARIABLE" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_OUTPUT_VARIABLE)
        message(FATAL_ERROR "Missing mandatory OUTPUT_VARIABLE argument.")
    endif()
    set(result "${ARG_VALUE}")
    string(REPLACE "\\" "\\\\" result "${result}")
    string(REPLACE "\"" "\\\"" result "${result}")
    set("${ARG_OUTPUT_VARIABLE}" "\"${result}\"" PARENT_SCOPE)
endfunction()

function(json_bool)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "VALUE;OUTPUT_VARIABLE" "")
    if (("OUTPUT_VARIABLE" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_OUTPUT_VARIABLE)
        message(FATAL_ERROR "Missing mandatory OUTPUT_VARIABLE argument.")
    endif()
    if (ARG_VALUE)
        set("${ARG_OUTPUT_VARIABLE}" "true" PARENT_SCOPE)
    else()
        set("${ARG_OUTPUT_VARIABLE}" "false" PARENT_SCOPE)
    endif()
endfunction()

json_bool(OUTPUT_VARIABLE json_color_diagnostics VALUE "${CMAKE_COLOR_DIAGNOSTICS}")
json_string(OUTPUT_VARIABLE json_cxx_flags VALUE "${CMAKE_CXX_FLAGS}")
json_string(OUTPUT_VARIABLE json_cxx_compiler VALUE "${CMAKE_CXX_COMPILER}")

block(SCOPE_FOR VARIABLES)
    set(svgViewer_path "$<IF:$<TARGET_EXISTS:svgViewer>,$<TARGET_FILE:svgViewer>,>")
    configure_file("cmake/cmake-variables.json.in" "cmake-variables.json.in")
    file(GENERATE OUTPUT "cmake-variables-$<CONFIG>.json" INPUT "${CMAKE_CURRENT_BINARY_DIR}/cmake-variables.json.in")
endblock()
