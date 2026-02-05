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
import shutil

import gustaveUtils as gu

class CreateMkdocsWorkdir(gu.GustaveScript):
    """Create the basic structure of MkDocs's working directory."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description="Create the basic structure of MkDocs's working directory")
        return result

    # @typing.override
    def doRun(self, ctx: gu.ScriptContext) -> None:
        mkdocsWorkdir = ctx.cmakeVars.folders.mkdocsWorkdir(checked = False)
        if os.path.exists(mkdocsWorkdir):
            shutil.rmtree(mkdocsWorkdir)
        os.makedirs(mkdocsWorkdir)
        os.makedirs(ctx.cmakeVars.folders.mkdocsGeneratedSnippets(checked = False))
        srcDir = ctx.cmakeVars.folders.source
        for item in ["README.md","docs","mkdocs.yml"]:
            source = os.path.join(srcDir, item)
            dest = os.path.join(mkdocsWorkdir, item)
            os.symlink(source, dest)



if __name__ == "__main__":
    CreateMkdocsWorkdir().run()
