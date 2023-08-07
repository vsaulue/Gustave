# This file is part of Gustave, a structural integrity library for video games.
# 
# Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

add_library(Comp-Utils INTERFACE)
target_include_directories(Comp-Utils INTERFACE "include")

add_executable(Test-Comp-Utils
    "tests/hash/HashEquals.cpp"
    "tests/hash/Hasher.cpp"
    "tests/hash/PointerHash.cpp"
    "tests/utils/getter.cpp"
    "tests/utils/NumberHyperscript.cpp"
    "tests/utils/SizedString.cpp"
    "tests/utils/SizedStringView.cpp"
)
target_include_directories(Test-Comp-Utils
    PRIVATE "tests/include"
)
target_link_libraries(Test-Comp-Utils
    PRIVATE Catch2::Catch2WithMain
    PRIVATE Comp-Utils
)
declare_unit_test(Test-Comp-Utils)
