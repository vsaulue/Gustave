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

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="installVcpkg", description="Install vcpkg.")
    parser.add_argument("--dest", required=True, type=pathlib.Path, help="Destination folder")
    args = parser.parse_args()

    os.makedirs(args.dest, exist_ok=True)

    cloneCmd = ["git", "clone", "https://github.com/microsoft/vcpkg.git", "."]
    subprocess.run(cloneCmd, cwd=args.dest)

    scriptName = "bootstrap-vcpkg.sh"
    if os.name == "nt":
        scriptName = "bootstrap-vcpkg.bat"

    bootstrapCmd = [os.path.join(args.dest, scriptName)]
    subprocess.run(bootstrapCmd, cwd=args.dest)
