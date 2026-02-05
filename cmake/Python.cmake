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

include("cmake/VariablesJson.cmake")

find_package(Python REQUIRED Interpreter)

add_custom_target(venvs
    COMMENT "Creating Python virtual environments for the project."
)

function(add_python_venv)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "TARGET" "")
    if (("TARGET" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Missing mandatory TARGET argument.")
    endif()

    set(venv_name "${ARG_TARGET}")
    set(venv_path "${CMAKE_BINARY_DIR}/venvs/${venv_name}")
    set(venv_requirements "${CMAKE_SOURCE_DIR}/cmake/venvs/${venv_name}/requirements.txt")

    add_custom_command(OUTPUT "${venv_path}/pyvenv.cfg"
        COMMAND
            Python::Interpreter "${python_scripts_dir}/createVenv.py"
            "--cmakeVariables" "${cmake_variables_json_neutral}"
            "--name" "${venv_name}"
        COMMENT "Rebuilding Python venv '${venv_name}' at '${venv_path}'"
        MAIN_DEPENDENCY
            "${venv_requirements}"
        DEPENDS
            "${python_scripts_dir}/createVenv.py"
            "${python_scripts_dir}/gustaveUtils.py"
    )

    add_custom_target("${venv_name}"
        DEPENDS "${venv_path}/pyvenv.cfg"
    )
    add_dependencies(venvs "${venv_name}")
endfunction()

function(add_custom_python_target)
    cmake_parse_arguments(PARSE_ARGV 0 ARG
        "ALL;USES_TERMINAL"
        "TARGET;VENV;COMMENT;WORKING_DIRECTORY"
        "COMMAND;DEPENDS"
    )
    if (("TARGET" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_TARGET)
        message(FATAL_ERROR "Missing mandatory TARGET argument.")
    endif()
    if (("VENV" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_VENV)
        message(FATAL_ERROR "Missing mandatory VENV argument.")
    endif()
    if(("COMMAND" IN_LIST ARG_KEYWORDS_MISSING_VALUES) OR NOT DEFINED ARG_COMMAND)
        message(FATAL_ERROR "Missing mandatory COMMAND argument.")
    endif()

    if(NOT TARGET "${ARG_VENV}")
        add_python_venv(TARGET "${ARG_VENV}")
    endif()

    set(other_args "")
    if(ARG_ALL)
        list(APPEND other_args "ALL")
    endif()
    if(ARG_USES_TERMINAL)
        list(APPEND other_args "USES_TERMINAL")
    endif()
    if(DEFINED ARG_COMMENT)
        list(APPEND other_args "COMMENT" "${ARG_COMMENT}")
    endif()
    if(DEFINED ARG_WORKING_DIRECTORY)
        list(APPEND other_args "WORKING_DIRECTORY" "${ARG_WORKING_DIRECTORY}")
    endif()

    add_custom_target("${ARG_TARGET}"
        ${other_args}
        COMMAND
            Python::Interpreter "${python_scripts_dir}/useVenv.py"
            "--cmakeVariables" "${cmake_variables_json}"
            "--venv" "${ARG_VENV}"
            "--" ${ARG_COMMAND}
        DEPENDS
            "${ARG_VENV}"
            ${ARG_DEPENDS}
    )
endfunction()
