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

import abc
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import typing

class DirectoryParser(object):
    """
    Argument parser for a directory path.

    It is meant to be used in `argparse.ArgumentParser.add_argument(type=DirectoryParser())`.
    """

    @staticmethod
    def isValid(arg: str) -> bool:
        """
        Checks if the argument is a diretory path.

        :param arg: string to test.
        :returns: true if `arg` is the path to a directory.
        """
        return os.path.isdir(arg)

    def __call__(self, arg: str) -> str:
        """
        Checks if the argument is a diretory path.

        :param arg: string to test.
        :returns: the input `arg`.
        :raises argparse.ArgumentTypeError: if the path isn't an existing directory.
        """
        if not os.path.isdir(arg):
            raise argparse.ArgumentTypeError('Invalid directory path.')
        return arg

class ExecutableParser(object):
    """
    Argument parser for a path to an executable.

    It is meant to be used in `argparse.ArgumentParser.add_argument(type=ExecutableParser())`.
    """

    @staticmethod
    def isValid(arg: str) -> bool:
        """
        Checks if the argument is a path to an executable.

        :param arg: string to test.
        :returns: true if `arg` is the path to an executable.
        """
        return os.path.isfile(arg) and os.access(arg, mode=os.X_OK)

    def __call__(self, arg: str) -> str:
        """
        Checks if the argument is a path to an executable.

        :param arg: string to test.
        :returns: the input `arg`.
        :raises argparse.ArgumentTypeError: if the file doesn't exist
            (or the file isn't executable).
        """
        if not os.path.isfile(arg):
            raise argparse.ArgumentTypeError('Invalid file path.')
        if not os.access(arg, mode= os.X_OK):
            raise argparse.ArgumentTypeError('The file must be executable.')
        return arg

class TtyColoring(object):
    """Base class to generate color for terminal output."""

    def __call__(self, text: str, colorName: str) -> str:
        """Colors a text with the specified color.

        :param text: the text to color.
        :param colorName: name of the color to use.
        :returns: the colored text.
        :raises ValueError: the color code doesn't exist.
        """
        raise NotImplementedError()

class NoColoring(TtyColoring):
    """Class disabling coloring of terminal text."""

    def __call__(self, text: str, colorName: str) -> str:
        return text

class AnsiColoring(TtyColoring):
    """Class doing coloring using ANSI escape sequences."""

    Codes : dict[str,str] = { 'green': '\033[92m', 'red': '\033[91m', 'yellow': '\033[93m' }
    """Codes[colorCode] -> ANSI_escapeSequence."""

    ResetCode : str = '\033[0m'
    """ANSI escape sequence to reset the color."""

    def __call__(self, text: str, colorName: str) -> str:
        if colorName not in self.Codes:
            raise ValueError('Unknown color code: "{0}"'.format(colorName))
        return '{0}{1}{2}'.format(self.Codes[colorName], text, self.ResetCode)

class Tty:
    """Static class grouping terminal functions."""

    IsAtty : bool = sys.stdout.isatty()
    """Flag indicating if the script is run in an interactive terminal."""

    @staticmethod
    def makeColoring(colourModeName: str) -> TtyColoring:
        """
        Factory for `TtyColoring` object, from command line arguments.

        :param colourModeName: the --colourMode argument.
        :returns: an instance of `TtyColoring` corresponding to the argument.
        :raises ValueError: if `colourModeName` is invalid.
        """
        if colourModeName == 'auto':
            if Tty.IsAtty:
                return AnsiColoring()
            else:
                return NoColoring()
        elif colourModeName == 'ansi':
            return AnsiColoring()
        elif colourModeName == 'none':
            return NoColoring()
        else:
            raise ValueError('Unknown colour mode: "{0}".'.format(colourModeName))


class JsonUtils(object):
    """Static class providing tools to parse JSON dictionaries."""

    @staticmethod
    def _getType(jsonObject: dict[str, typing.Any], name: str, expType: typing.Type) -> typing.Any:
        """
        Gets the object at the specified key, and performs a type check.

        :param jsonObject: Object containing the key/value.
        :param name: Key of the desired value.
        :param expType: Expected type for the return value.
        :returns: the object at `jsonObject[name]`.
        :raises TypeError: if the value isn't of `expType` type.
        """
        result = jsonObject[name]
        if not isinstance(result, expType):
            raise TypeError(f'Expected type "{expType}" for field "{name}" (found type : "{type(result)}").')
        return result

    @classmethod
    def getBool(cls, jsonObject: dict[str, typing.Any], name: str) -> bool:
        """
        Gets the bool value at the specified key.

        :param jsonObject: Object containing the key/value.
        :param name: Key of the desired value.
        :returns: the object at `jsonObject[name]`.
        :raises TypeError: if the value isn't of `bool` type.
        """
        return cls._getType(jsonObject, name, bool)

    @classmethod
    def getObject(cls, jsonObject: dict[str, typing.Any], name: str) -> dict[str, typing.Any]:
        """
        Gets the JSON object value at the specified key.

        :param jsonObject: Object containing the key/value.
        :param name: Key of the desired value.
        :returns: the object at `jsonObject[name]`.
        :raises TypeError: if the value isn't of `dict` type.
        """
        return cls._getType(jsonObject, name, dict)

    @classmethod
    def getStr(cls, jsonObject: dict[str, typing.Any], name: str) -> str:
        """
        Gets the string value at the specified key.

        :param jsonObject: Object containing the key/value.
        :param name: Key of the desired value.
        :returns: the object at `jsonObject[name]`.
        :raises TypeError: if the value isn't of `str` type.
        """
        return cls._getType(jsonObject, name, str)



class CMakeBuildFlags(object):
    """Class holding the build flags used in the current Gustave build."""

    _cxx: str
    """Value of the `cxx` property."""

    def __init__(self, jsonObject: dict[str, typing.Any]):
        """
        :param jsonObject: Input JSON sub-object.
        :raises TypeError: if an unexpected type is found in the JSON.
        """
        self._cxx = JsonUtils.getStr(jsonObject, 'cxx')

    @property
    def cxx(self) -> str:
        """Build flags passed to the c++ compiler."""
        return self._cxx



class CMakeCompilers(object):
    """Class holding the compilers used in the current Gustave build."""

    _cxx : str
    """Value of the `cxx` property."""

    _cxxChecked : bool
    """Flag set when `_cxx` was checked to be a path to an executable."""


    def __init__(self, jsonObject: dict[str, typing.Any]):
        """
        :param jsonObject: Input JSON sub-object.
        :raises TypeError: if an unexpected type is found in the JSON.
        """
        self._cxx = JsonUtils.getStr(jsonObject, 'cxx')
        self._cxxChecked = False

    @property
    def cxx(self) -> str:
        """
        Path to the c++ compiler (gcc/clang/cl.exe ...).

        :raises ValueError: The value is not the path to an executable.
        """
        result = self._cxx
        if not self._cxxChecked:
            if not ExecutableParser.isValid(result):
                raise ValueError(f'"cxx" property is not a valid executable: {result}.')
            self._cxxChecked = True
        return result



class CMakeExecutables(object):
    """Class holding the executables used/produced in the current Gustave build."""

    _cmake : str
    """Value of the `cmake` property."""

    _cmakeChecked : bool
    """Flag set when `_cmake` was checked to be a path to an executable."""

    _compilers : CMakeCompilers
    """Value of the `compilers` property."""

    _conan : str
    """Value of the `conan` property."""

    _conanChecked : bool
    """Flag set when `_conan` was checked to be a path to an executable."""

    _drMemory : str
    """Value of the `drMemory` property."""

    _drMemoryChecked : bool
    """Flag set when `_drMemory` was checked to be a path to an executable."""

    _svgViewer : str
    """Value of the `svgViewer` property."""

    _svgViewerChecked : bool
    """Flag set when `_conan` was checked to be a path to an executable."""

    _valgrind : str
    """Value of the `valgrind` property."""

    _valgrindChecked : bool
    """Flag set when `_valgrind` was checked to be a path to an executable."""

    def __init__(self, jsonObject: dict[str, typing.Any]):
        """
        :param jsonObject: Input JSON sub-object.
        :raises TypeError: if an unexpected type is found in the JSON.
        """
        self._cmake = JsonUtils.getStr(jsonObject, 'cmake')
        self._cmakeChecked = False
        self._compilers = CMakeCompilers(JsonUtils.getObject(jsonObject, 'compilers'))
        self._conan = JsonUtils.getStr(jsonObject, 'conan')
        self._conanChecked = False
        self._drMemory = JsonUtils.getStr(jsonObject, 'drMemory')
        self._drMemoryChecked = False
        self._svgViewer = JsonUtils.getStr(jsonObject, 'svgViewer')
        self._svgViewerChecked = False
        self._valgrind = JsonUtils.getStr(jsonObject, 'valgrind')
        self._valgrindChecked = False

    @property
    def cmake(self) -> str:
        """
        Path to the cmake executable.

        :raises ValueError: The value is not the path to an executable.
        """
        result = self._cmake
        if not self._cmakeChecked:
            if not ExecutableParser.isValid(result):
                raise ValueError(f'"cmake" property is not a valid executable: {result}.')
            self._cmakeChecked = True
        return result

    @property
    def compilers(self) -> CMakeCompilers:
        """Path to compiler executables."""
        return self._compilers

    @property
    def conan(self) -> str:
        """
        Path to the conan executable.

        :raises ValueError: The value is not the path to an executable.
        """
        result = self._conan
        if not self._conanChecked:
            if not ExecutableParser.isValid(result):
                raise ValueError(f'"conan" property is not a valid executable: {result}.')
            self._conanChecked = True
        return result

    @property
    def drMemory(self) -> str:
        """
        Path to the drMemory executable.

        :raises ValueError: The value is not the path to an executable.
        """
        result = self._drMemory
        if not self._drMemoryChecked:
            if not ExecutableParser.isValid(result):
                raise ValueError(f'"drMemory" property is not a valid executable: {result}.')
            self._drMemoryChecked = True
        return result

    @property
    def svgViewer(self):
        """
        Path to the svgViewer executable (built in Gustave).

        :raises ValueError: The value is not the path to an executable.
        """
        result = self._svgViewer
        if not self._svgViewerChecked:
            if not ExecutableParser.isValid(result):
                raise ValueError(f'"svgViewer" property is not a valid executable: {result}.')
            self._svgViewerChecked = True
        return result

    @property
    def valgrind(self):
        """
        Path to the valgrind executable.

        :raises ValueError: The value is not the path to an executable.
        """
        result = self._valgrind
        if not self._valgrindChecked:
            if not ExecutableParser.isValid(result):
                raise ValueError(f'"valgrind" property is not a valid executable: {result}.')
            self._valgrindChecked = True
        return result

class CMakeFolders(object):
    """Class holding specific folders in the current Gustave build."""

    _build : str
    """Value of the `build` property."""

    _buildChecked : bool
    """Flag set when `_build` was checked to be a path to a folder."""

    _install : str
    """Value of the `install` property."""

    _installChecked : str
    """Flag set when `_install` was checked to be a path to a folder."""

    _source : str
    """Value of the `source` property."""

    _sourceChecked : bool
    """Flag set when `_source` was checked to be a path to a folder."""

    _testPackageChecked: bool
    """Flag set when `testPackage` was checked to be a path to a folder."""

    def __init__(self, jsonObject: dict[str, typing.Any]):
        """
        :param jsonObject: Input JSON sub-object.
        :raises TypeError: if an unexpected type is found in the JSON.
        """
        self._build = JsonUtils.getStr(jsonObject, 'build')
        self._buildChecked = False
        self._install = JsonUtils.getStr(jsonObject, 'install')
        self._installChecked = False
        self._source = JsonUtils.getStr(jsonObject, 'source')
        self._sourceChecked = False
        self._testPackageChecked = False

    @property
    def build(self) -> str:
        """
        Path to CMake's build folder.

        :raises ValueError: The value is not the path to a folder.
        """
        result = self._build
        if not self._buildChecked:
            if not DirectoryParser.isValid(result):
                raise ValueError(f'"build" property is not a valid directory: {result}.')
            self._buildChecked = True
        return result

    @property
    def install(self) -> str:
        """
        Path to CMake's install folder.

        :raises ValueError: The value is not the path to a folder.
        """
        result = self._install
        if not self._installChecked:
            if not DirectoryParser.isValid(result):
                raise ValueError(f'"install" property is not a valid directory: {result}.')
            self._installChecked = True
        return result

    @property
    def source(self) -> str:
        """
        Path to the project's source folder.

        :raises ValueError: The value is not the path to a folder.
        """
        result = self._source
        if not self._sourceChecked:
            if not DirectoryParser.isValid(result):
                raise ValueError(f'"source" property is not a valid directory: {result}.')
            self._sourceChecked = True
        return result

    @property
    def testPackage(self) -> str:
        """
        Path to the project's test_package folder.

        :raises ValueError: The value is not the path to a folder.
        """
        result = os.path.join(self._source, "packaging", "test_package")
        if not self._testPackageChecked:
            if not DirectoryParser.isValid(result):
                raise ValueError(f'"testPackage" property is not a valid directory: {result}.')
            self._testPackageChecked = True
        return result

class CMakeConanProfiles(object):
    """Class holding the name/path of the conan profiles."""

    _build: str
    """Value of the `build` property."""

    _host: str
    """Value of the `host` property."""

    def __init__(self, jsonObject: dict[str, typing.Any]):
        """
        :param jsonObject: Input JSON sub-object.
        :raises TypeError: if an unexpected type is found in the JSON.
        """
        self._build = JsonUtils.getStr(jsonObject, 'build')
        self._host = JsonUtils.getStr(jsonObject, 'host')

    @property
    def build(self) -> str:
        """Name (or path) of the Conan build profile."""
        result = self._build
        if result == '':
            raise ValueError('"build" property is empty.')
        return result

    @property
    def host(self) -> str:
        """Name (or path) of the Conan host profile."""
        result = self._host
        if result == '':
            raise ValueError('"host" property is empty.')
        return result

class CMakeVariables(object):
    """Class holding various values provided by CMake in the current Gustave build."""

    _buildFlags : CMakeBuildFlags
    """Value of the `buildFlags` property."""

    _colorDiagnostics: bool
    """Value of the `colorDiagnostics` property."""

    _conanProfiles: CMakeConanProfiles
    """Value of the `conanProfiles` property."""

    _config : str
    """Value of the `config` property."""

    _executables : CMakeExecutables
    """Value of the `executables` property."""

    _folders : CMakeFolders
    """Value of the `folders` property."""

    _memcheckType : str
    """Value of the `memcheckType` property."""

    def __init__(self, jsonObject: dict[str, typing.Any]):
        """
        :param jsonObject: Input JSON sub-object.
        :raises TypeError: if an unexpected type is found in the JSON.
        """
        self._buildFlags = CMakeBuildFlags(JsonUtils.getObject(jsonObject, 'buildFlags'))
        self._colorDiagnostics = JsonUtils.getBool(jsonObject, 'colorDiagnostics')
        self._conanProfiles = CMakeConanProfiles(JsonUtils.getObject(jsonObject, 'conanProfiles'))
        self._config = JsonUtils.getStr(jsonObject, 'config')
        self._executables = CMakeExecutables(JsonUtils.getObject(jsonObject, 'executables'))
        self._folders = CMakeFolders(JsonUtils.getObject(jsonObject, 'folders'))
        self._memcheckType = JsonUtils.getStr(jsonObject, 'memcheckType')

    @property
    def buildFlags(self) -> CMakeBuildFlags:
        """The build flags used in the Gustave project."""
        return self._buildFlags

    @property
    def colorDiagnostics(self) -> bool:
        """Flag set when CMake is with color diagnostics (CMAKE_COLOR_DIAGNOSTICS)."""
        return self._colorDiagnostics

    @property
    def conanProfiles(self) -> CMakeConanProfiles:
        return self._conanProfiles

    @property
    def config(self) -> str:
        """Current CMake config (Debug/Release/...)."""
        return self._config

    @property
    def executables(self) -> CMakeExecutables:
        """Exectables provided by CMake."""
        return self._executables

    @property
    def folders(self) -> CMakeFolders:
        """Directories provided by CMake."""
        return self._folders

    @property
    def memcheckType(self) -> str:
        return self._memcheckType

class TestScriptCommand(object):
    """Class to run an external program and store its results."""

    _completedProcess: subprocess.CompletedProcess
    """Result of the underlying `subprocess.run()` call."""

    _outPath : str | None
    """Value of the `outPath` property."""

    _separateStderr : bool
    """Value of the `separateStderr` property."""

    _stderrPath : str | None
    """(Optional) path to the file storing the program's standard error."""

    def __init__(self, cmd: list[str], separateStderr = False):
        """
        :param cmd: Program name & arguments.
        :param separateStderr: True to store the command's stdout & stderr into separate files.
        """
        self._outPath = None
        self._stderrPath = None
        self._separateStderr = separateStderr
        try:
            outFile = tempfile.NamedTemporaryFile('w+', delete = False)
            self._outPath = outFile.name
            errFile = outFile
            if separateStderr:
                errFile = tempfile.NamedTemporaryFile('w+', delete = False)
                self._stderrPath = errFile.name
            self._completedProcess = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=outFile, stderr=errFile)
        except:
            self.close()
            raise

    def __enter__(self) -> 'TestScriptCommand':
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    @property
    def returncode(self) -> int:
        """Return code of the program run."""
        return self._completedProcess.returncode

    @property
    def cmd(self) -> list[str]:
        """Full command (program name + arguments) used to run the program."""
        return self._completedProcess.args

    @property
    def cmdStr(self) -> str:
        """Single string holding the command (program name + argument). NOT ESCAPED."""
        return " ".join(self._completedProcess.args)

    @property
    def outPath(self) -> str:
        """Path to the output file (containing the command's stdout, and optionally stderr)."""
        result = self._outPath
        if result == None:
            raise ValueError('outPath is not available anymore.')
        return result

    @property
    def separateStderr(self) -> bool:
        """
        Flag indicating if the command's stdout and stderr are stored into separate files.

        If False, the `outPath` contains both stdout and stderr. `stderrPath` is invalid.
        If True, the `outPath` contains stdout. The `stderrPath` contains stderr.
        """
        return self._separateStderr

    @property
    def stderrPath(self) -> str:
        """
        Path to the file storing the stderr's command.

        Only available if `separateStderr` was set to True.
        """
        result = self._stderrPath
        if result == None:
            raise ValueError('stderrPath is not available anymore.')
        return result

    def close(self):
        """Releases all resources held by this object."""
        try:
            if self._outPath != None:
                os.remove(self._outPath)
                self._outPath = None
        except Exception:
            pass
        try:
            if self._stderrPath != None:
                os.remove(self._stderrPath)
                self._stderrPath = None
        except Exception:
            pass

class TestScriptException(Exception):
    """Exception thrown when a TestScriptCommand failed unexpectedly."""

    _errCode: int
    """Value of the `errCode` property."""

    def __init__(self, errCode: int = 1):
        """
        :param errCode: Error code of the failed command.
        """
        super().__init__(f'Test script failed with error code {errCode}.')
        self._errCode = errCode

    @property
    def errCode(self) -> int:
        """Error code returned by the failed program."""
        return self._errCode

class TestScriptContext(object):
    """Class holding various data/utility useful to run Gustave tests."""

    _args: argparse.Namespace
    """Value of the `args` property."""

    _cmakeVars: CMakeVariables | None
    """(Optional) value of the `cmakeVars` property."""

    _coloring: TtyColoring | None
    """Value of the `coloring` property (lazy generation)."""

    _tmpFolder: str | None
    """Value of the `tmpFolder` property (lazy generation)."""

    def __init__(self, cmakeVars: CMakeVariables | None, args: argparse.Namespace):
        """
        :param cmakeVars: Value of the `cmakeVars` property.
        :param args: Parsed command line for the test script.
        """
        self._args = args
        self._cmakeVars = cmakeVars
        self._coloring = None
        self._tmpFolder = None

    @property
    def args(self) -> argparse.Namespace:
        """Parsed command line arguments."""
        return self._args

    @property
    def cmakeVars(self) -> CMakeVariables:
        """Variables parsed from the current CMake build."""
        result = self._cmakeVars
        if result == None:
            raise ValueError('cmakeVars was not defined.')
        return result

    @property
    def coloring(self) -> TtyColoring:
        """Coloring object to use to generate colored stderr messages."""
        result = self._coloring
        if result == None:
            colorModeName = 'auto'
            if self._cmakeVars != None:
                colorModeName = 'ansi' if self._cmakeVars.colorDiagnostics else 'none'
            result = Tty.makeColoring(colorModeName)
            self._coloring = result
        return result

    def newCmd(self, cmd: list[str], exitOnError : bool = True, separateStderr : bool = False) -> TestScriptCommand:
        """
        Runs an external program.

        It should be used in a `with` statement to ensure that the result's `close()` method is properly called.

        :param cmd: program name/path and command line arguments.
        :param exitOnError: name of the color to use.
        :param separateStderr: True if stdout & stderr should be stored into separate file.
        :returns: the result of the program run.
        :raises TestScriptException: if the program returned an error and `exitOnError` is True.
        """
        cmdInfoPrinted = self.verboseLevel > 0
        if cmdInfoPrinted:
            print(f'{self.coloring("Running command", "yellow")}: {" ".join(cmd)}', file = sys.stderr)
        result = TestScriptCommand(cmd, separateStderr=separateStderr)
        if (result.returncode != 0) or (self.verboseLevel > 1):
            errPath = result.stderrPath if separateStderr else result.outPath
            if os.path.getsize(errPath) > 0:
                if not cmdInfoPrinted:
                    print(f'{self.coloring("Running command", "yellow")}: {" ".join(cmd)}', file = sys.stderr)
                print('--------------------', file=sys.stderr)
                with open(errPath) as errFile:
                    shutil.copyfileobj(fsrc = errFile, fdst = sys.stderr)
                print('--------------------\n', file=sys.stderr)
        if result.returncode != 0 and exitOnError:
            print(f'{self.coloring("Command FAILED", "red")} (exit code: {result.returncode}): {result.cmdStr}', file=sys.stderr)
            result.close()
            raise TestScriptException(result.returncode)
        return result

    def runCmd(self, cmd: list[str], exitOnError : bool = True, separateStderr : bool = False) -> None:
        """
        Runs an external program.

        :param cmd: program name/path and command line arguments.
        :param exitOnError: name of the color to use.
        :param separateStderr: True if stdout & stderr should be stored into separate file.
        :raises TestScriptException: if the program returned an error and `exitOnError` is True.
        """
        with self.newCmd(cmd, exitOnError, separateStderr):
            pass

    @property
    def tmpFolder(self) -> str:
        """Get an automatically managed temporary directory."""
        result = self._tmpFolder
        if result == None:
            result = tempfile.mkdtemp()
            self._tmpFolder = result
        return result

    @property
    def verboseLevel(self) -> int:
        """Verbosity level of the test script."""
        result = self._args.verbose
        if result == None:
            result = 0
        return result

    def cleanup(self) -> None:
        """Deletes the temporary directory."""
        if self._tmpFolder != None:
            shutil.rmtree(self._tmpFolder)

class TestScript(abc.ABC):
    """Class to run a test in Gustave's build."""

    def useCMakeVars(self) -> bool:
        """
        Indicates if the script uses the '--cmakeVariables' argument.

        :returns: True to use `cmakeVars`.
        """
        return True

    @abc.abstractmethod
    def makeArgsParser(self) -> argparse.ArgumentParser:
        """
        Generates the ArgumentParser instance to use in this script.

        :returns: The argument parser to use.
        """
        raise NotImplementedError()

    @abc.abstractmethod
    def doRun(self, ctx: TestScriptContext) -> None:
        """
        Internal implementation of the test.

        :param ctx: Script's context.
        """
        raise NotImplementedError()

    def run(self) -> None:
        """
        Runs the full test.

        This method parses the command line arguments, read build info from CMake, and calls `self.doRun()`.
        """
        argsParser = self.makeArgsParser()
        if self.useCMakeVars():
            argsParser.add_argument('--cmakeVariables', required=True, type=argparse.FileType(), help='CMake generated JSON file containing the project variables.')
        argsParser.add_argument('--verbose', '-v', action='count', default=None, help='Verbosity level.')
        args = argsParser.parse_args()
        cmakeVars = None
        if self.useCMakeVars():
            cmakeVars = CMakeVariables(json.load(args.cmakeVariables))
        ctx = TestScriptContext(cmakeVars, args)
        try:
            self.doRun(ctx)
        except TestScriptException as e:
            print(f'{ctx.coloring("Test FAILED", "red")}: {argsParser.prog}', file=sys.stderr)
            sys.exit(e.errCode)
        ctx.cleanup()
