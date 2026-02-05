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

block(SCOPE_FOR VARIABLES)
    set(Gustave_FOUND "TRUE" PARENT_SCOPE)

    set(known_components "Distrib-Std")
    foreach(component_name IN LISTS Gustave_FIND_COMPONENTS)
        set(Gustave_${component_name}_FOUND "FALSE" PARENT_SCOPE)
        if(NOT component_name IN_LIST known_components)
            if(Gustave_FIND_REQUIRED_${component_name})
                message(SEND_ERROR "Gustave: unknown REQUIRED component: ${component_name}")
                set(Gustave_FOUND "FALSE" PARENT_SCOPE)
            else()
                message(WARNING "Gustave: unknown optional component: ${component_name}")
            endif()
        endif()
    endforeach()

    # Component Distrib-Std
    set(Gustave_Distrib-Std_FOUND "TRUE" PARENT_SCOPE)
    include("${CMAKE_CURRENT_LIST_DIR}/Distrib-Std-targets.cmake")
endblock()
