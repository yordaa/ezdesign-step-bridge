# ezd2step

`ezd2step` converts proprietary EzDesign `.ezd` geometry files to STEP. This
repository owns the product source, version in `CMakeLists.txt`, release tags,
tests, and binary releases.

OCCT and nlohmann-json are external dependencies installed by vcpkg. Neither
OCCT nor vcpkg is vendored here.

## Build and test

Prerequisites: CMake 3.20+, a C++17 compiler, and a vcpkg checkout.

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=arm64-osx-dynamic \
  -DVCPKG_OVERLAY_TRIPLETS="$PWD/triplets" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On Windows x64, omit the overlay and use
`-DVCPKG_TARGET_TRIPLET=x64-windows`.

```sh
build/ezd2step input.ezd output.step
```

## Release

Tags named `v<version>` are the version authority and must match the version in
`CMakeLists.txt`. The release workflow builds macOS arm64 and Windows x64
bundles and publishes them to the matching GitHub release.

## Registry

Binary consumers use the separate private
[`yordaa/ezdesign-vcpkg-registry`](https://github.com/yordaa/ezdesign-vcpkg-registry)
Git registry. Its consumer example contains the required
`vcpkg.json` and `vcpkg-configuration.json`.

## Migration

This repository previously vendored all of OCCT and contained its own vcpkg
registry. The registry moved to `yordaa/ezdesign-vcpkg-registry`; OCCT and
nlohmann-json now come from vcpkg. A vcpkg checkout is local build tooling, not
a maintained repository.

## License

The ezd2step source is proprietary. See `LICENSE` and `NOTICE`. Distributed
OCCT libraries remain subject to OCCT's LGPL-2.1 terms.
