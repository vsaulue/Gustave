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

import argparse
import sys

import gustaveUtils as gu

class RunMemcheckTest(gu.TestScript):
    """Tests that the CMake config files from the install folder works correctly."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description='Checks that the cmake Config files in the install folder works.')
        result.add_argument('command', nargs='*', help='Program/arguments to instrument/run in memcheck')
        return result

    # @typing.override
    def doRun(self, ctx: gu.TestScriptContext) -> None:
        if len(ctx.args.command) == 0:
            print(f'{ctx.coloring("ERROR","red")}: No program name & arguments provided.', file=sys.stderr)
            raise gu.TestScriptException()
        cmd = None
        memcheckType = ctx.cmakeVars.memcheckType
        if memcheckType == 'valgrind':
            cmd = [ ctx.cmakeVars.executables.valgrind, '--error-exitcode=1', '--leak-check=full', '--' ] + ctx.args.command
        elif memcheckType == 'drMemory':
            cmd = [ ctx.cmakeVars.executables.drMemory, '-exit_code_if_errors', '1', '-batch', '--' ] + ctx.args.command
        else:
            print(f'{ctx.coloring("ERROR","red")}: Unknown "memcheckType" value: {memcheckType}.', file=sys.stderr)

        ctx.runCmd(cmd, separateStderr=True)


if __name__ == "__main__":
    RunMemcheckTest().run()
