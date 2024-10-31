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

add_library(Distrib-Std-Unitless INTERFACE)
target_include_directories(Distrib-Std-Unitless INTERFACE "include")
install(DIRECTORY "include/" COMPONENT Distrib-Std DESTINATION "distrib-std/include")

target_link_libraries(Distrib-Std-Unitless
    INTERFACE Comp-Core
    INTERFACE Comp-Math
    INTERFACE Comp-StdUnitless
)

add_library(Testconfig-Core-Std-Unitless INTERFACE)
target_include_directories(Testconfig-Core-Std-Unitless INTERFACE "tests/include")
target_link_libraries(Testconfig-Core-Std-Unitless
    INTERFACE Distrib-Std-Unitless
)

add_test_core(Test-Core-Std-Unitless Testconfig-Core-Std-Unitless)
