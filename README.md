# ezdesign-step-bridge

`ezdesign-step-bridge` builds the `ezd2step` CLI, which converts proprietary
EzDesign `.ezd` geometry files to STEP. This
repository owns the product source, version in `CMakeLists.txt`, release tags,
CLI contract, and behavior tests.

OCCT and nlohmann-json are external dependencies installed by vcpkg. The
shared registry supplies the minimal OCCT port; neither OCCT nor vcpkg is
vendored here.

## Build and test

Prerequisites: CMake 3.20+, a C++17 compiler, and a vcpkg checkout.

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=arm64-osx-dynamic \
  -DVCPKG_OVERLAY_TRIPLETS="$PWD/triplets" \
  -DCMAKE_INSTALL_PREFIX="$PWD/build/vcpkg_installed/arm64-osx-dynamic" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On Windows x64, use `-DVCPKG_TARGET_TRIPLET=x64-windows-release` and the
matching install prefix.

```sh
build/vcpkg_installed/arm64-osx-dynamic/bin/ezd2step input.ezd output.step
```

## Release

Tags named `v<version>` are the version authority and must match the versions in
`CMakeLists.txt` and `vcpkg.json`. Tagged source archives are the release artifact.

## Registry

Source builds use the separate public
[`yordaa/ezdesign-vcpkg-registry`](https://github.com/yordaa/ezdesign-vcpkg-registry)
Git registry. This repository selects its OCCT port through
`vcpkg-configuration.json`; the registry consumer package is named
`ezdesign-step-bridge`.

## Migration

This repository previously vendored all of OCCT and contained its own vcpkg
registry. The registry moved to `yordaa/ezdesign-vcpkg-registry`; OCCT and
nlohmann-json now come from vcpkg. A vcpkg checkout is local build tooling, not
a maintained repository.

## License

The ezdesign-step-bridge source is proprietary. See `LICENSE` and `NOTICE`. Distributed
OCCT libraries remain subject to OCCT's LGPL-2.1 terms.
