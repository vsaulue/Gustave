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
import os
import pathlib
import subprocess
import sys
import tempfile

class TestConfig(object):
    ScriptFolder = pathlib.Path(__file__).parent.resolve()
    WorldFolder = os.path.join(ScriptFolder, 'jsonSamples', 'worlds')
    RendererFolder = os.path.join(ScriptFolder, 'jsonSamples', 'renderers')

    DefaultWorldPath = os.path.join(WorldFolder, 'tower10.json')

class TtyColors:
    Green = '\033[92m'
    Red = '\033[91m'
    Reset = '\033[0m'

class ExecutableParser(object):
    def __init__(self):
        pass

    def __call__(self, arg: str) -> str:
        if not os.path.isfile(arg):
            raise argparse.ArgumentTypeError('Invalid file path.')
        if not os.access(arg, mode= os.X_OK):
            raise argparse.ArgumentTypeError('The file must be executable.')
        return arg

class TestContext(object):
    IsAtty = sys.stdout.isatty()

    def __init__(self, args: list[str]):
        parser = argparse.ArgumentParser(prog='testSvgViewer', description='Runs automated tests for svgViewer and jsonSamples.')
        parser.add_argument('svgViewerPath', type=ExecutableParser(), help='Path of the svgViewer binary to test')
        parser.add_argument('--colour-mode', dest='colorMode', choices=['ansi','none','auto'], default='auto', help='Color mode used in the standard output')
        args = parser.parse_args(args)
        self.svgViewerPath = args.svgViewerPath
        if args.colorMode == 'ansi':
            self._useAnsiColor = True
        elif args.colorMode == 'none':
            self._useAnsiColor = False
        else:
            self._useAnsiColor = self.IsAtty

    def printDelimiter(self, char: str):
        print(char * 40)

    def printLine(self, *msg: str):
        print(*msg)

    def textColor(self, msg: str, colorCode: str):
        if self._useAnsiColor:
            return '{0}{1}{2}'.format(colorCode, msg, TtyColors.Reset)
        else:
            return msg

class TestReport(object):
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
    def __init__(self, name: str, context: TestContext):
        self.name = name
        self.context = context

    def commandList(self, filename: str) -> list[str]:
        raise NotImplementedError()

    def folderPath(self) -> str:
        raise NotImplementedError()

    def fullCasePath(self, filename) -> str:
        return os.path.join(self.folderPath(), filename)

    def listCases(self) -> list[str]:
        return [ f for f in os.listdir(self.folderPath()) if self.fullCasePath(f) ]

    def run(self) -> TestReport:
        ctx = self.context
        allCases = self.listCases()
        numCases = len(allCases)
        curCaseId = 0
        numSuccessCases = 0
        numFailedCases = 0
        failedText = ctx.textColor('FAILED', TtyColors.Red)

        ctx.printDelimiter('=')
        print('Running "{0}" samples folder ({1} files found)...'.format(self.name, numCases))
        ctx.printDelimiter('-')
        for case in allCases:
            curCaseId += 1
            with tempfile.TemporaryFile(mode='w+') as stderrFile:
                cmd = self.commandList(case)
                viewerProcess = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=stderrFile)
                if viewerProcess.returncode != 0:
                    numFailedCases += 1
                    ctx.printLine("[{0}/{1}] {2}: {3}".format(curCaseId, numCases, failedText, ' '.join(cmd)))
                    stderrFile.seek(0)
                    ctx.printLine(stderrFile.read())
                    ctx.printDelimiter('-')
                else:
                    numSuccessCases += 1
        return TestReport(numSuccessCases, numFailedCases)

class WorldsFolder(TestFolder):
    def __init__(self, context: TestContext):
        super().__init__('worlds', context)

    def folderPath(self) -> str:
        return TestConfig.WorldFolder

    def commandList(self, filename) -> list[str]:
        return [ self.context.svgViewerPath, self.fullCasePath(filename) ]

class RenderersFolder(TestFolder):
    def __init__(self, context: TestContext):
        super().__init__('renderers', context)

    def folderPath(self) -> str:
        return TestConfig.RendererFolder

    def commandList(self, filename) -> list[str]:
        return [ self.context.svgViewerPath, TestConfig.DefaultWorldPath, '-r', self.fullCasePath(filename) ]

if __name__ == "__main__":
    ctx = TestContext(sys.argv[1:])
    folders = [ WorldsFolder(ctx), RenderersFolder(ctx) ]
    reports = TestReport(0,0)
    for folder in folders:
        reports = reports + folder.run()
    if reports.numFailedCases == 0:
        ctx.printLine(ctx.textColor('All tests passed ({0} cases)'.format(reports.numSuccessCases), TtyColors.Green))
        sys.exit(0)
    else:
        passedText = ctx.textColor('{0} passed'.format(reports.numSuccessCases), TtyColors.Green)
        failedText = ctx.textColor('{0} failed'.format(reports.numFailedCases), TtyColors.Red)
        ctx.printLine('Test cases: {0} | {1} | {2}'.format(reports.numCases(), passedText, failedText))
        sys.exit(1)

