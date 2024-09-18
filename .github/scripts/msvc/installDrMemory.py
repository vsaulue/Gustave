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
import urllib.request
import shutil
import sys
import tempfile
import zipfile

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog='installDrMemory', description='Install DrMemory for windows.')
    parser.add_argument('--dest', required=True, type=pathlib.Path, help='Destination folder')
    args = parser.parse_args()

    zipPath, _ = urllib.request.urlretrieve('https://github.com/DynamoRIO/drmemory/releases/download/release_2.6.0/DrMemory-Windows-2.6.0.zip')
    os.makedirs(args.dest, exist_ok=True)
    with zipfile.ZipFile(zipPath) as zipRef:
        zipRef.extractall(args.dest)
    urllib.request.urlcleanup()

    unzippedPath = os.path.join(args.dest, 'DrMemory-Windows-2.6.0')
    shutil.move(os.path.join(unzippedPath, 'bin64'), args.dest)
    shutil.move(os.path.join(unzippedPath, 'dynamorio'), args.dest)
    shutil.rmtree(unzippedPath)

    # Warm-up run (Initialize symcache in a non-concurrent environment)
    drMemoryPath = os.path.join(args.dest, 'bin64', 'drmemory.exe')
    if not os.access(drMemoryPath, mode= os.X_OK):
        print('Installation failed: no executable at: {0}'.format(drMemoryPath), file=sys.stderr)
        sys.exit(1)
    print('DrMemory installed at: {0}'.format(drMemoryPath))
    with tempfile.TemporaryFile(mode='w+') as stderrFile:
        testRun = subprocess.run([drMemoryPath, '-batch', '--', 'cmd', '/c', 'echo Test program!'], stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=stderrFile)
        if testRun.returncode != 0:
            stderrFile.seek(0)
            print(stderrFile.read(), file=sys.stderr)
            print('DrMemory warm-up run FAILED.', file=sys.stderr)
        else:
            print('DrMemory warm-up run OK.')
        sys.exit(testRun.returncode)
