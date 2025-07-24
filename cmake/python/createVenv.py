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

import argparse
import os

import gustaveUtils as gu

class MakeVenv(gu.TestScript):
    """Reinstall a virtual environment required by the build system."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description="")
        result.add_argument("--name", type=str, required=True, help="Name of the new virtual environment")
        return result

    # @typing.override
    def doRun(self, ctx: gu.TestScriptContext) -> None:
        venvsSource = ctx.cmakeVars.folders.venvsSource
        requirementPath = os.path.join(venvsSource, ctx.args.name, "requirements.txt")
        if not os.path.isfile(requirementPath):
            raise ValueError(f'Cannot build venv "{ctx.args.name}": no requirements at "{requirementPath}".')
        newEnv = ctx.venvs.create(ctx.args.name)
        newEnv.runCmd(["python", "-m", "pip", "install", "-r", requirementPath])


if __name__ == "__main__":
    MakeVenv().run()
