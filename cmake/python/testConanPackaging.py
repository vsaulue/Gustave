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
import json
import re
import typing

import gustaveUtils as gu

class TestConanPackaging(gu.TestScript):
    """Tests that the conanfile.py packaging works."""

    @staticmethod
    def _extractGustaveRef(jsonObject: dict[str,typing.Any]) -> str:
        nodes = jsonObject['graph']['nodes']
        regex = re.compile(r'^gustave/')
        result = None
        for key,value in nodes.items():
            tmpResult = value['ref']
            res = regex.match(tmpResult)
            if res != None:
                result = tmpResult
                break
        if result == None:
            raise ValueError('gustave dependency not found in the JSON object.')
        return result

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description='Checks that the packaging using the local conanfile works.')
        return result

    # @typing.override
    def doRun(self, ctx: gu.TestScriptContext) -> None:
        conanExe = ctx.cmakeVars.executables.conan

        createCmd = [ conanExe, 'create', ctx.cmakeVars.folders.source,
            f'-pr:b={ctx.cmakeVars.conanProfiles.build}',
            f'-pr:h={ctx.cmakeVars.conanProfiles.host}',
            '-s:h', 'build_type={0}'.format(ctx.cmakeVars.config),
            '-c', 'tools.build:skip_test=True',
            '--build=missing',
            '--format=json'
        ]
        with ctx.newCmd(createCmd, separateStderr=True) as createRun:
            with open(createRun.outPath) as stdoutFile:
                createJson = json.load(stdoutFile)

        # Cleanup
        gustavePackageRef = self._extractGustaveRef(createJson)
        cleanupCmd = [ conanExe, 'remove', gustavePackageRef, '--confirm' ]
        ctx.runCmd(cleanupCmd, exitOnError=False)

if __name__ == "__main__":
    TestConanPackaging().run()
