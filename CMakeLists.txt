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

cmake_minimum_required(VERSION 3.29)

set(gustave_version "0.1.0")

project("Gustave" VERSION "${gustave_version}" LANGUAGES "CXX")
set(CMAKE_CXX_STANDARD 20)

# General settings
option(CMAKE_COMPILE_WARNING_AS_ERROR "Compile warnings as errors." ON)
option(GUSTAVE_BUILD_TOOLS "Build internal tools (set this to OFF if you just want to package)." ON)
option(GUSTAVE_BUILD_TUTORIALS "Build example executables (set this to OFF if you just want to package)." ON)
# CMAKE_COLOR_DIAGNOSTICS: Add colors to build/test output.
# CMAKE_INSTALL_PREFIX: Default location of the install target (also used for packaging tests).

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set_property(CACHE CMAKE_INSTALL_PREFIX PROPERTY VALUE "${CMAKE_CURRENT_BINARY_DIR}/install")
endif()

# General test settings
# BUILD_TESTING: build tests (set this to OFF if you just want to package).
option(GUSTAVE_ENABLE_MEMCHECK_TESTS "Enable memcheck tests (ignored if BUILD_TESTING is OFF)." ON)
option(GUSTAVE_ENABLE_PACKAGING_TESTS "Enable packaging tests (ignored if BUILD_TESTING is OFF)." ON)
option(GUSTAVE_ENABLE_UNIT_TESTS "Enable unit tests (ignored if BUILD_TESTING is OFF)." ON)

# Memcheck tests settings
set(GUSTAVE_MEMCHECKER_PATH CACHE PATH "Path containing the memcheck program (valgrind, DrMemory).")

# Packaging tests settings
set(GUSTAVE_CONAN_PATH CACHE PATH "Path to conan program (for packaging tests).")
set(GUSTAVE_CONAN_BUILD_PROFILE CACHE STRING "Conan build profile (for packaging tests).")
set(GUSTAVE_CONAN_HOST_PROFILE CACHE STRING "Conan host profile (for packaging tests).")
set(GUSTAVE_VCPKG_ROOT CACHE STRING "Path to the vcpkg installation (for packaging tests).")

find_package(Python COMPONENTS Interpreter)

set(python_scripts_dir "${CMAKE_CURRENT_SOURCE_DIR}/cmake/python")
set(json_samples_dir "${CMAKE_CURRENT_SOURCE_DIR}/tools/jsonSamples")

include("CMakePackageConfigHelpers")
include("CTest")

include("cmake/CompilerOptions.cmake")
include("cmake/MemcheckTests.cmake")
include("cmake/PackagingTests.cmake")
include("cmake/ToolTests.cmake")
include("cmake/Tutorials.cmake")
include("cmake/UnitTests.cmake")
include("cmake/VariablesJson.cmake")

add_custom_target(install-to-cmake-prefix
    COMMAND "${CMAKE_COMMAND}" "--build" "${CMAKE_CURRENT_BINARY_DIR}" "--config" "$<CONFIG>" "--target" "install"
)

add_subdirectory("components")
add_subdirectory("distribs")
add_subdirectory("docs")
add_subdirectory("packaging")
add_subdirectory("tools")
