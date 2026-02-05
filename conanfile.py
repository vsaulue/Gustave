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

from conan import ConanFile
from conan.tools.build import can_run, check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import re
import os

class GustaveRecipe(ConanFile):
    name = "gustave"
    license = "MIT"
    author = "Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>"
    url = "https://github.com/vsaulue/Gustave"
    description = "A structural integrity library for video games"
    topics = ("physics", "structural-integrity", "game-development", "header-library")

    settings = "os", "arch", "compiler", "build_type"

    exports = "LICENSE.txt"
    exports_sources = "cmake/*", "components/*", "distribs/*", "docs/*", "packaging/*", "tools/*", "CMakeLists.txt", "!*/build", "!*/__pycache__"
    test_package_folder = "packaging/test_package"

    def _memcheckTestsEnabled(self) -> bool:
        return self._testsEnabled() or self.conf.get("user.gustave:enable_memcheck_tests", default=True, check_type=bool)

    def _packagingTestsEnabled(self) -> bool:
        return self._testsEnabled() or self.conf.get("user.gustave:enable_packaging_tests", default=True, check_type=bool)

    def _docsEnabled(self) -> bool:
        return self.conf.get("user.gustave:build_docs", default=True, check_type=bool)

    def _testsEnabled(self) -> bool:
        return self.conf.get("tools.build:skip_test", default=False)

    def _toolsEnabled(self) -> bool:
        return self.conf.get("user.gustave:build_tools", default=True, check_type=bool)

    def _tutorialsEnabled(self) -> bool:
        return self.conf.get("user.gustave:build_tutorials", default=True, check_type=bool)

    def _unitTestsEnabled(self) -> bool:
        return self._testsEnabled() or self.conf.get("user.gustave:enable_unit_tests", default=True, check_type=bool)

    def set_version(self):
        cmakeFilePath = os.path.join(self.recipe_folder, 'CMakeLists.txt')
        regex = re.compile(r'^ *set\( *gustave_version +"(\d+\.\d+\.\d+)" *\)')
        with open(cmakeFilePath) as cmakeFile:
            for line in cmakeFile:
                res = regex.match(line)
                if res != None:
                    self.version = res.group(1)
                    break

    def build_requirements(self):
        self.tool_requires("cmake/3.29.0")
        if self._unitTestsEnabled():
            self.test_requires("catch2/3.6.0")
        if self._toolsEnabled():
            self.test_requires("nlohmann_json/3.11.3")
            self.test_requires("svgwrite/0.2.0")
        if self._toolsEnabled() or self._tutorialsEnabled():
            self.test_requires("cli11/2.4.2")

    def validate(self):
        check_min_cppstd(self, 20)

    def layout(self):
        self.folders.build_folder_vars = ["settings.arch", "settings.os", "settings.compiler", "settings.compiler.version"]
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["GUSTAVE_BUILD_DOCS"] = self._docsEnabled()
        toolchain.cache_variables["GUSTAVE_BUILD_TOOLS"] = self._toolsEnabled()
        toolchain.cache_variables["GUSTAVE_BUILD_TUTORIALS"] = self._tutorialsEnabled()
        toolchain.cache_variables["GUSTAVE_ENABLE_MEMCHECK_TESTS"] = self._memcheckTestsEnabled()
        toolchain.cache_variables["GUSTAVE_ENABLE_PACKAGING_TESTS"] = self._packagingTestsEnabled()
        toolchain.cache_variables["GUSTAVE_ENABLE_UNIT_TESTS"] = self._unitTestsEnabled()
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if can_run(self):
            cmake.ctest(cli_args=["-LE", "packaging-test"])

    def package(self):
        copy(self, "LICENSE.txt", src=str(self.recipe_folder), dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install(component="Distrib-Std")

    def package_info(self):
        std = self.cpp_info.components['distrib-std']
        std.libdirs = []
        std.bindirs = []
        std.includedirs = ['distrib-std/include']
        std.set_property('cmake_file_name', 'Gustave')
        std.set_property('cmake_target_name', 'Gustave::Distrib-Std')

        # Disable the generation of the default 'gustave::gustave' target by giving it an existing name.
        self.cpp_info.set_property('cmake_file_name', 'Gustave')
        self.cpp_info.set_property('cmake_target_name', 'Gustave::Distrib-Std')

    def package_id(self):
        self.info.clear()
