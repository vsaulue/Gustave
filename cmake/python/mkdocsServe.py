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
import pathlib

import gustaveUtils as gu

class MkdocsServe(gu.GustaveScript):
    """Launch `mkdocs --serve` for this project's documentation."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description="Launch `mkdocs --serve` for this project's documentation")
        result.add_argument("otherArgs", type=str, nargs="*", help="Additional arguments to pass to Mkdocs")
        return result

    # @typing.override
    def doRun(self, ctx: gu.ScriptContext) -> None:
        env = ctx.venvs.get("venv-mkdocs")
        workdir = ctx.cmakeVars.folders.mkdocsWorkdir()
        cmd = ["mkdocs", "serve", "--livereload", "-w", workdir] + ctx.args.otherArgs
        env.runCmd(cmd, exitOnError=False, io=gu.ForwardIO(), cwd=workdir)


if __name__ == "__main__":
    MkdocsServe().run()
