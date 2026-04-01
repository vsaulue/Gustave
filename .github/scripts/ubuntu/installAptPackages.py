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
import re
import shutil
import subprocess
import tempfile

class ToolchainId:
    _osName: str
    _compilerName: str
    _compilerVersion: int

    def __init__(self, idString: str):
        regex = re.compile(r"^(\w+)-(\w+)-(\d+)$")
        res = regex.match(idString)
        if res is None:
            raise ValueError(f"Invalid toolchain id: '{idString}'")
        self._osName = res.group(1)
        self._compilerName = res.group(2)
        self._compilerVersion = int(res.group(3))

    @property
    def osName(self) -> str:
        return self._osName

    @property
    def compilerName(self) -> str:
        return self._compilerName

    @property
    def compilerVersion(self) -> int:
        return self._compilerVersion

def runLlvmScript(version: int) -> None:
    homeDir = pathlib.Path.home()
    scriptPath = os.path.join(homeDir, "llvm.sh")
    subprocess.run(["wget", "-P", homeDir, "--", "https://apt.llvm.org/llvm.sh"], check=True)
    subprocess.run(["chmod", "u+x", scriptPath], check=True)
    subprocess.run(["sudo", scriptPath, str(version)], check=True)

def installXpackGcc(dirName: str, tarUrl: str) -> None:
    homeDir = pathlib.Path.home()
    compilerDir = os.path.join(homeDir, dirName)
    shutil.rmtree(compilerDir, ignore_errors=True)
    os.makedirs(compilerDir)
    with tempfile.TemporaryDirectory() as tmpDir:
        tarFilepath = os.path.join(tmpDir, "gcc-tarball")
        subprocess.run(["wget", "-O", tarFilepath, tarUrl], check=True)
        subprocess.run(["tar", "-xvzf", tarFilepath, "-C", compilerDir, "--strip-components=1"], check=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="installAptPackages", description="Install Apt packages and the C++ compiler for builds on Ubuntu.")
    parser.add_argument("--toolchainId", required=True, type=ToolchainId, help="Toolchain identifier (os-compilerName-compilerVersion)")
    args = parser.parse_args()

    chainId: ToolchainId = args.toolchainId
    if chainId.osName != "linux":
        raise ValueError(f"Unsupported OS: {chainId.osName}")

    subprocess.run(["sudo", "apt", "update"], check=True)

    compilerName = chainId.compilerName
    compilerVersion = chainId.compilerVersion
    aptPackageNames : list[str] = ["valgrind", "ninja-build"]
    if compilerName == "gcc":
        if compilerVersion == 13:
            pass
        elif compilerVersion == 14:
            pass
        elif compilerVersion == 15:
            installXpackGcc("xpack-gcc-15", "https://github.com/xpack-dev-tools/gcc-xpack/releases/download/v15.2.0-1/xpack-gcc-15.2.0-1-linux-x64.tar.gz")
        else:
            raise ValueError(f"Unsupported {compilerName} version: {compilerVersion}")
    elif compilerName == "clang":
        if compilerVersion == 17:
            aptPackageNames.append("libc++-17-dev")
        elif compilerVersion == 18:
            aptPackageNames.append("libc++-18-dev")
        elif compilerVersion > 18:
            runLlvmScript(compilerVersion)
            aptPackageNames.append(f"libc++-{compilerVersion}-dev")
        else:
            raise ValueError(f"Unsupported {compilerName} version: {compilerVersion}")
    else:
        raise ValueError(f"Unsupported compiler: {compilerVersion}")
    subprocess.run(["sudo", "apt", "install", "--"] + aptPackageNames, check=True)
