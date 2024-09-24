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
import sys

class DirectoryParser(object):
    """
    Argument parser for a directory path.

    It is meant to be used in `argparse.ArgumentParser.add_argument(type=DirectoryParser())`.
    """

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

    Codes : dict[str,str] = { 'green': '\033[92m', 'red': '\033[91m' }
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
