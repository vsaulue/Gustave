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

from conan import ConanFile
from conan.tools.build import can_run, check_min_cppstd
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os

class GustaveRecipe(ConanFile):
    name = "gustave"
    version = "0.0.1"
    license = "MIT"
    author = "Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>"
    url = "https://github.com/vsaulue/Gustave"
    description = "A structural integrity library for video games"
    topics = ("physics", "structural-integrity", "game-development", "header-library")

    settings = "os", "arch", "compiler", "build_type"

    exports = "LICENSE.txt"
    exports_sources = "cmake/*", "components/*", "distribs/*", "tools/*", "CMakeLists.txt"

    def build_requirements(self):
        self.tool_requires("cmake/3.29.0")
        self.test_requires("catch2/3.6.0")
        self.test_requires("cli11/2.4.2")
        self.test_requires("nlohmann_json/3.11.3")
        self.test_requires("svgwrite/0.2.0")

    def validate(self):
        check_min_cppstd(self, 20)

    def layout(self):
        self.folders.build_folder_vars = ["settings.arch", "settings.os", "settings.compiler", "settings.compiler.version"]
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        toolchain = CMakeToolchain(self)
        if self.conf.get("user.gustave:disable_cmake_user_preset", default=False):
            toolchain.user_presets_path = False
        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        if not self.conf.get("tools.build:skip_test", default=False):
            cmake.build()
            if can_run(self):
                cmake.build(target="run-unit-tests")

    def package(self):
        copy(self, "LICENSE.txt", src=str(self.recipe_folder), dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install(component="Std-Gustave")

    def package_info(self):
        std = self.cpp_info.components['distrib-std']
        std.libdirs = []
        std.bindirs = []
        std.includedirs = ['include']
        std.set_property('cmake_file_name', 'Gustave')
        std.set_property('cmake_target_name', 'Gustave::Distrib-Std')

        # Disable the generation of the default 'gustave::gustave' target by giving it an existing name.
        self.cpp_info.set_property('cmake_file_name', 'Gustave')
        self.cpp_info.set_property('cmake_target_name', 'Gustave::Distrib-Std')

    def package_id(self):
        self.info.clear()
