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

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/gustave-config-version.cmake"
    COMPATIBILITY SameMinorVersion
    ARCH_INDEPENDENT
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/gustave-config-version.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/gustave-config.cmake"
    COMPONENT Packaging-CMake
    DESTINATION "cmake"
)

install(
    TARGETS Distrib-Std
    EXPORT Distrib-Std-targets
    COMPONENT Distrib-Std
    RUNTIME
        DESTINATION "bin"
    LIBRARY
        DESTINATION "lib"
    ARCHIVE
        DESTINATION "lib"
    FILE_SET HEADERS
        DESTINATION "include"
)
install(
    EXPORT Distrib-Std-targets
    COMPONENT Packaging-CMake
    NAMESPACE Gustave::
    DESTINATION "cmake"
)

add_packaging_test(TEST_ID cmake-packaging-test
    COMMAND Python::Interpreter "${python_scripts_dir}/testCMakePackaging.py"
        "--cmakeVariables" "${cmake_variables_json}"
    WORKING_DIRECTORY "${Gustave_SOURCE_DIR}"
    DEPENDS_ON install-to-cmake-prefix
)
