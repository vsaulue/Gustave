# Installation

This section explains the recommended ways to include the Gustave library into your project.

Make sure your project is built with C++20 (or later) enabled.

## Conan + CMake

### Conan center (available SOON™)

Official releases of Gustave will be available on the [Conan center](https://conan.io/center).

Add the gustave dependency in your project's conanfile.txt:
```
[requires]
gustave/0.1.0
```

Usage in your project's CMakeLists.txt:
```
find_package(Gustave CONFIG REQUIRED Distrib-Std)

add_executable(your-binary
    "main.cpp"
)
target_link_libraries(your-binary PRIVATE Gustave::Distrib-Std)
```

### From source

The [source tree](https://github.com/vsaulue/Gustave) has a valid conanfile that can be used to create the package locally on your machine.

```
git clone https://github.com/vsaulue/Gustave.git
conan create ./Gustave -s:h build_type=Release -c tools.build:skip_test=True --build=missing
```

This package can then be consumed normally by your other local Conan projects.

## Vcpkg

### From official registry (available SOON™)

### From source

Gustave's [source tree](https://github.com/vsaulue/Gustave/packaging/vcpkg/overlay) has a vcpkg overlay that can be used by your projects locally.

**Note:** if you're using Visual Studio, use a [standalone installation of vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg) (the one shipped with the IDE seems to have issues with overlays).

1. Clone Gustave somewhere on your machine (and optionally, checkout a specific version):
```
git clone https://github.com/vsaulue/Gustave.git /home/user123/somewhere/gustave
```

2. Add an [overlay port](https://learn.microsoft.com/en-us/vcpkg/concepts/overlay-ports) in the [vcpkg-configuration.json](https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-configuration-json) file of your project, to `packaging/vcpkg/overlay` inside Gustave. Example:
```
{
    "default-registry": {
        "kind": "git",
        "baseline": "1a21d756f26d6b974cf52544b291bcdc65de6f4c",
        "repository": "https://github.com/microsoft/vcpkg"
    },
    "overlay-ports": [
        "/home/user123/somewhere/Gustave/packaging/vcpkg/overlay"
    ]
}

```

3. Add a dependency to Gustave in the [vcpkg.json](https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-json) of your project:
```
{
    "dependencies": [
        "gustave"
    ]
}
```

## CMake

The release builds includes proper CMake packaging files in the `cmake/` subfolder. To use them:

1. Download a `gustave-release.zip` file from the [Github releases](https://github.com/vsaulue/Gustave/releases).
1. Unzip it somewhere on your machine.
1. Edit your project's `CMakeLists.txt` as follows:
```
find_package(Gustave CONFIG REQUIRED Distrib-Std)

add_executable(your-binary
    "main.cpp"
)
target_link_libraries(your-binary PRIVATE Gustave::Distrib-Std)
```
4. When configuring your project, add `-D Gustave_DIR=/path/to/unzipped/gustave-release/cmake` to your project's command line (note the `cmake` subfolder).

## Other

Gustave is currently a header-only library. If you don't use Conan or CMake, the release builds package all headers in a single folder:

1. Download a `gustave-release.zip` file from the [Github releases](https://github.com/vsaulue/Gustave/releases).
1. Unzip it somewhere on your machine.
1. Add the subfolder `distrib-std/include` of the unzipped `gustave-release` as a header directory.

For example, to compile directly with `gcc` command's line:
```
g++ -std=c++20 -I /path/to/unzipped/gustave-release/distrib-std/include my-program.cpp -o my-program
```
