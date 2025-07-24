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

include("cmake/Python.cmake")

set(mkdocs_working_dir "${CMAKE_SOURCE_DIR}")
set(mkdocs_output_dir "${CMAKE_BINARY_DIR}/docs/mkdocs-build")

if(GUSTAVE_BUILD_DOCS)
    add_custom_python_target(TARGET mkdocs-build ALL
        VENV venv-mkdocs
        COMMAND "mkdocs" "build" "-d" "${mkdocs_output_dir}" "-s"
        WORKING_DIRECTORY "${mkdocs_working_dir}"
        COMMENT "Building documentation at '${mkdocs_output_dir}'"
    )

    add_custom_python_target(TARGET mkdocs-serve
        VENV venv-mkdocs
        COMMAND "mkdocs" "serve"
        WORKING_DIRECTORY "${mkdocs_working_dir}"
        COMMENT "Serving documentation at 'http://localhost:8000' (!!! WILL RUN UNTIL KILLED !!!)"
        USES_TERMINAL
    )

    add_custom_python_target(TARGET mike-deploy
        VENV venv-mkdocs
        COMMAND "mike" "deploy" "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        WORKING_DIRECTORY "${mkdocs_working_dir}"
        COMMENT "Deploying documentation to git branch 'gh-pages'"
    )

    install(DIRECTORY "${mkdocs_output_dir}/"
        DESTINATION mkdocs
        COMPONENT MkDocs
    )
endif()
