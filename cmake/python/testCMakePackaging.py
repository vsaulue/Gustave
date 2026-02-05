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

import argparse
import os

import gustaveUtils as gu

class TestCMakePackaging(gu.GustaveScript):
    """Tests that the CMake config files from the install folder works correctly."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description='Checks that the cmake Config files in the install folder works.')
        return result

    # @typing.override
    def doRun(self, ctx: gu.ScriptContext) -> None:
        testProjectPath = os.path.join(ctx.cmakeVars.folders.source, 'packaging', 'test_package')
        gustaveConfigPath = os.path.join(ctx.cmakeVars.folders.install, 'cmake')
        buildFolder = ctx.tmpFolder

        configCmd = [ ctx.cmakeVars.executables.cmake,
            '-B', buildFolder, '-S', testProjectPath,
            '-D', f'Gustave_DIR={gustaveConfigPath}',
            '-D', f'CMAKE_BUILD_TYPE={ctx.cmakeVars.config}',
            '-D', f'CMAKE_CXX_FLAGS={ctx.cmakeVars.buildFlags.cxx}',
            '-D', f'CMAKE_CXX_COMPILER={ctx.cmakeVars.executables.compilers.cxx}',
            '-D', f'CMAKE_COLOR_DIAGNOSTICS={ctx.cmakeVars.colorDiagnostics}'
        ]
        ctx.runCmd(configCmd)

        buildCmd = [ ctx.cmakeVars.executables.cmake, '--build', buildFolder, '--target', 'run-test' ]
        ctx.runCmd(buildCmd)
        

if __name__ == "__main__":
    TestCMakePackaging().run()
