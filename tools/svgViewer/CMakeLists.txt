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

add_executable(svgViewer
    "main.cpp"
)
target_link_libraries(svgViewer
    PRIVATE CLI11::CLI11
    PRIVATE Distrib-Std-StrictUnit
    PRIVATE Examples-JsonWorld
)

add_tool_test(TEST_ID svgViewer-tool-test
    COMMAND Python::Interpreter "${python_scripts_dir}/testSvgViewer.py"
        "--cmakeVariables" "${cmake_variables_json}"
    DEPENDS svgViewer
)

add_memcheck_test(
    TEST_ID "svgViewer-01"
    TARGET svgViewer
    ARGS "${json_samples_dir}/worlds/tower10.json"
)
add_memcheck_test(
    TEST_ID "svgViewer-02"
    TARGET svgViewer
    ARGS
        "-r" "${json_samples_dir}/renderers/blockMaxStress.json"
        "${json_samples_dir}/worlds/tower10.json"
)
