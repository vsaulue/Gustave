# Installation

This section explains the recommended ways to include the Gustave library into a C++ project.

Make sure the project is built with C++20 (or later).

## Conan

### Conan center (SOON™)

Official releases of Gustave will be available on the [Conan center](https://conan.io/center).

Add the gustave dependency in the project's conanfile.txt:
```
[requires]
gustave/0.1.900
```

Usage in the project's CMakeLists.txt:
```
find_package(Gustave CONFIG REQUIRED Distrib-Std)

add_executable(new-executable
    "main.cpp"
)
target_link_libraries(new-executable PRIVATE Gustave::Distrib-Std)
```

### From source

The [source tree](https://github.com/vsaulue/Gustave) has a valid conanfile that can be used to create the package locally.

```
git clone https://github.com/vsaulue/Gustave.git
conan create ./Gustave -s:h build_type=Release -c tools.build:skip_test=True -c user.gustave:build_tutorials=False -c user.gustave:build_docs=False -c user.gustave:build_tools=False --build=missing
```

!!! note
    Repeat the last command line for all `build_type` used in the project (`Debug`, `RelWithDebInfo`, ...)

This package can then be consumed normally by other local Conan projects.

## Vcpkg

### Official registry (SOON™)

### From source

Gustave's [source tree](https://github.com/vsaulue/Gustave/packaging/vcpkg/overlay) has a vcpkg overlay that can be used by other projects locally.

!!! note
    for Visual Studio, use a [standalone installation of vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg) (the one shipped with the IDE seems to have issues with overlays).

1. Clone Gustave (and optionally, checkout a specific version):

    ```
    git clone https://github.com/vsaulue/Gustave.git /home/user123/somewhere/Gustave
    ```

1. Add an [overlay port](https://learn.microsoft.com/en-us/vcpkg/concepts/overlay-ports) in the [vcpkg-configuration.json](https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-configuration-json) file of the project, to `packaging/vcpkg/overlay` inside Gustave. Example:

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

1. Add a dependency to Gustave in the [vcpkg.json](https://learn.microsoft.com/en-us/vcpkg/reference/vcpkg-json) of the project:

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
1. Unzip it somewhere.
1. Edit the project's `CMakeLists.txt` as follows:
    ```
    find_package(Gustave CONFIG REQUIRED Distrib-Std)

    add_executable(new-binary
        "main.cpp"
    )
    target_link_libraries(new-binary PRIVATE Gustave::Distrib-Std)
    ```
4. When configuring the project, add `-D Gustave_DIR=/path/to/unzipped/gustave-release/cmake` in CMake's command line (note the `cmake` subfolder).

## Other

Gustave is currently a header-only library. The release builds package all headers in a single folder for easy integration with other build systems:

1. Download a `gustave-release.zip` file from the [Github releases](https://github.com/vsaulue/Gustave/releases).
1. Unzip it somewhere.
1. Add the subfolder `distrib-std/include` of the unzipped `gustave-release` as a header directory into the build.

For example, to compile directly with `gcc`'s command line:
```
g++ -std=c++20 -I /path/to/unzipped/gustave-release/distrib-std/include my-program.cpp -o my-program
```
