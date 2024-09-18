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
import subprocess
import sys
import tempfile

import gustaveUtils as gu

def fatalError(msg : str) -> None:
    print(msg, file=sys.stderr)
    sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='runMemcheckTest', description='Runs memcheck (valgrind/drMemory) on a test program.')
    parser.add_argument('command', nargs='*', help='Test program & arguments to run')
    parser.add_argument('--colour-mode', dest='colorMode', choices=['ansi','none','auto'], default='auto', help='Color mode used in the standard output')
    parser.add_argument('--valgrind', type=gu.ExecutableParser(), help='Path to the valgrind executable')
    parser.add_argument('--drMemory', type=gu.ExecutableParser(), help='Path to the drMemory executable')
    args = parser.parse_args()
    coloring = gu.Tty.makeColoring(args.colorMode)

    if len(args.command) == 0:
        fatalError('No test program provided.')
    cmd = None
    if args.valgrind != None:
        cmd = [ args.valgrind, '--error-exitcode=1', '--leak-check=full', '--' ] + args.command
    if args.drMemory != None:
        if cmd != None:
            fatalError('"--valgrind" and "--drMemory" cannot be used at the same time.')
        cmd = [ args.drMemory, '-exit_code_if_errors', '1', '-batch', '--' ] + args.command
    if cmd == None:
        fatalError('Missing mandatory option: "--valgrind" or "--drMemory".')

    failedText = coloring('Test FAILED', 'red')
    with tempfile.TemporaryFile(mode='w+') as stderrFile:
        memcheckRun = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=stderrFile)
        if memcheckRun.returncode != 0:
            stderrFile.seek(0)
            print(stderrFile.read(), file=sys.stderr)
            print('{0}: {1}'.format(failedText, ' '.join(cmd)), file=sys.stderr)
        sys.exit(memcheckRun.returncode)
