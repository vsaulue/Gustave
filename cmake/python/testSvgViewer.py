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

import abc
import argparse
import os
import sys

import gustaveUtils as gu

class TestConfig(object):
    JsonSamplesPath: list[str] = ['tools','jsonSamples']
    WorldSubFolder: str = 'worlds'
    RendererSubFolder: str = 'renderers'
    DefaultWorldFilename: str = 'tower10.json'

class TestReport(object):
    numSuccessCases: int
    numFailedCases: int

    def __init__(self, numSuccessCases: int, numFailedCases: int):
        self.numSuccessCases = numSuccessCases
        self.numFailedCases = numFailedCases

    def __add__(self, other: 'TestReport') -> 'TestReport':
        numSuccessCases = self.numSuccessCases + other.numSuccessCases
        numFailedCases = self.numFailedCases + other.numFailedCases
        return TestReport(numSuccessCases, numFailedCases)

    def numCases(self) -> int:
        return self.numSuccessCases + self.numFailedCases


class TestFolder(object):
    _name: str
    _ctx: gu.ScriptContext
    _casesFolderPath : str

    def __init__(self, name: str, ctx: gu.ScriptContext):
        self._name = name
        self._ctx = ctx
        self._casesFolderPath = os.path.join(ctx.cmakeVars.folders.source, *TestConfig.JsonSamplesPath, name)

    @abc.abstractmethod
    def commandList(self, filename: str) -> list[str]:
        raise NotImplementedError()

    def fullCasePath(self, filename: str) -> str:
        return os.path.join(self._casesFolderPath, filename)

    def listCases(self) -> list[str]:
        return [ f for f in os.listdir(self._casesFolderPath) if self.fullCasePath(f) ]

    @property
    def svgViewerPath(self):
        return self._ctx.cmakeVars.executables.svgViewer

    def run(self) -> TestReport:
        allCases = self.listCases()
        numCases = len(allCases)
        numSuccessCases = 0
        numFailedCases = 0

        print('=' * 40, file=sys.stderr)
        print(f'Running "{self._name}" samples folder ({numCases} files found)...\n', file=sys.stderr)
        for case in allCases:
            cmd = self.commandList(case)
            with self._ctx.newCmd(cmd, exitOnError=False, io=gu.SeparateIO()) as caseRun:
                retCode = caseRun.returncode
                if retCode != 0:
                    print(f'{self._ctx.coloring("Command failed", "red")} (exit code: {retCode}): {caseRun.cmdStr}', file=sys.stderr)
                    numFailedCases += 1
                else:
                    numSuccessCases += 1
        return TestReport(numSuccessCases, numFailedCases)

class WorldsFolder(TestFolder):
    def __init__(self, ctx: gu.ScriptContext):
        super().__init__(TestConfig.WorldSubFolder, ctx)

    # @typing.override
    def commandList(self, filename) -> list[str]:
        return [ self.svgViewerPath, self.fullCasePath(filename) ]

class RenderersFolder(TestFolder):
    _defaultWorldPath : str

    def __init__(self, ctx: gu.ScriptContext):
        super().__init__(TestConfig.RendererSubFolder, ctx)
        self._defaultWorldPath = os.path.join(ctx.cmakeVars.folders.source, *TestConfig.JsonSamplesPath, TestConfig.WorldSubFolder, TestConfig.DefaultWorldFilename)

    # @typing.override
    def commandList(self, filename) -> list[str]:
        return [ self.svgViewerPath, self._defaultWorldPath, '-r', self.fullCasePath(filename) ]

class TestSvgViewer(gu.GustaveScript):
    """Tests the svgViewer executable and the JSON samples."""

    # @typing.override
    def makeArgsParser(self) -> argparse.ArgumentParser:
        result = argparse.ArgumentParser(description='Checks that svgViewer and the examples in jsonSamples/ work.')
        return result

    # @typing.override
    def doRun(self, ctx: gu.ScriptContext) -> None:
        folders = [ WorldsFolder(ctx), RenderersFolder(ctx) ]
        reports = TestReport(0,0)
        for folder in folders:
            reports = reports + folder.run()
        if reports.numFailedCases == 0:
            print(ctx.coloring(f'All tests passed ({reports.numSuccessCases} cases)', 'green'), file=sys.stderr)
        else:
            passedText = ctx.coloring('{0} passed'.format(reports.numSuccessCases), 'green')
            failedText = ctx.coloring('{0} failed'.format(reports.numFailedCases), 'red')
            print(f'Test cases: {reports.numCases()} | {passedText} | {failedText}', file=sys.stderr)
            raise gu.ScriptException()

if __name__ == "__main__":
    TestSvgViewer().run()
