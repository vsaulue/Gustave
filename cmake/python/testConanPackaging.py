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
import subprocess
import sys
import tempfile

import gustaveUtils as gu

def _extractGustaveRef(jsonObject: json) -> str:
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

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='testConanPackaging', description='Checks that conan packaging works.')
    parser.add_argument('sourceDir', help='Project base directory (containing conanfile.py)')
    parser.add_argument('--colour-mode', dest='colorMode', choices=['ansi','none','auto'], default='auto', help='Color mode used in the standard output')
    parser.add_argument('--build-type', dest='buildType', choices=['Debug','Release','RelWithDebInfo'], required=True, help='Build type')
    parser.add_argument('--conan', required=True, type=gu.ExecutableParser(), help='Conan executable')
    parser.add_argument('--pr-build', dest='prBuild', required=True, help='Conan build profile')
    parser.add_argument('--pr-host', dest='prHost', required=True, help='Conan host profile')
    args = parser.parse_args()
    coloring = gu.Tty.makeColoring(args.colorMode)

    cmd = [ args.conan, 'create', args.sourceDir,
        '-pr:b={0}'.format(args.prBuild),
        '-pr:h={0}'.format(args.prHost),
        '-s:h', 'build_type={0}'.format(args.buildType),
        '-c', 'tools.build:skip_test=True',
        '--build=missing',
        '--format=json'
    ]
    failedText = coloring('Test FAILED', 'red')
    with tempfile.TemporaryFile(mode='w+') as stdoutFile:
        with tempfile.TemporaryFile(mode='w+') as stderrFile:
            conanCreate = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=stdoutFile, stderr=stderrFile)
            if conanCreate.returncode != 0:
                stderrFile.seek(0)
                print(stderrFile.read(), file=sys.stderr)
                print('{0}: {1}'.format(failedText, ' '.join(cmd)), file=sys.stderr)
                sys.exit(1)
        stdoutFile.seek(0)
        createJson = json.load(stdoutFile)

    # Cleanup
    gustavePackageRef = _extractGustaveRef(createJson)
    cleanupCmd = [ args.conan, 'remove', gustavePackageRef, '--confirm' ]
    subprocess.run(cleanupCmd, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
