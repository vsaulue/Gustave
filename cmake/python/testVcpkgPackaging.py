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
import json
import os
import re
import shutil
import typing

import gustaveUtils as gu

class TestVcpkgPackaging(gu.GustaveScript):
    """Tests that the local vcpkg portfile works."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description="Checks that the packaging using the local vcpkg portfile works.")
        return result

    # @typing.override
    def doRun(self, ctx: gu.ScriptContext) -> None:
        tmpTestPackagePath = os.path.join(ctx.tmpFolder, "test_package")
        cmakeExe = ctx.cmakeVars.executables.cmake

        shutil.copytree(ctx.cmakeVars.folders.testPackage, tmpTestPackagePath)

        configuredFolderPath = os.path.join(ctx.cmakeVars.folders.build, "packaging", "vcpkg", "test_package")
        shutil.copytree(src=configuredFolderPath, dst=tmpTestPackagePath, dirs_exist_ok=True)

        configCmd = [ cmakeExe, "--preset", "vcpkg", "-D", f'CMAKE_BUILD_TYPE={ctx.cmakeVars.config}']
        ctx.runCmd(configCmd, cwd=tmpTestPackagePath)

        buildCmd = [ cmakeExe, "--build", "--preset", "vcpkg", "--target", "run-test"]
        ctx.runCmd(buildCmd, cwd=tmpTestPackagePath)

if __name__ == "__main__":
    TestVcpkgPackaging().run()
