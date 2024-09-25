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

cmake_minimum_required (VERSION 3.24)

project ("Gustave" VERSION 0.0.1 LANGUAGES "CXX")
set(CMAKE_CXX_STANDARD 20)

set(GUSTAVE_MEMCHECKER_PATH CACHE PATH "Path containing the memcheck program (valgrind, DrMemory).")

find_package(Catch2 CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(svgwrite CONFIG REQUIRED)
find_package(Python COMPONENTS Interpreter)

set(python_scripts_dir "${CMAKE_CURRENT_SOURCE_DIR}/cmake/python")
set(json_samples_dir "${CMAKE_CURRENT_SOURCE_DIR}/tools/jsonSamples")

include("cmake/MemcheckTests.cmake")
include("cmake/UnitTests.cmake")

add_custom_target(run-tool-tests)

add_subdirectory("components")
add_subdirectory("distribs")
add_subdirectory("tools")
