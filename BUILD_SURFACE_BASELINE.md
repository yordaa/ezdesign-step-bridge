# ezd2step build-surface baseline

Measurements for [issue #10](https://github.com/yordaa/ezdesign-step-bridge/issues/10), captured at commit `71c5fd842b` on 2026-07-25.

## Runner

- macOS 26.5 arm64, Apple M4 (10 cores)
- CMake 4.3.4
- Apple Clang 21.0.0
- vcpkg `6d332a018c433fad20822ff4b536e4ccdc3413bd`

## Vendored Release baseline

The documented clean build was run with:

```sh
cmake -S . -B <empty-build-dir> \
  -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_MINIMAL_DISTRIBUTION=ON \
  -DBUILD_MINIMAL_PROFILE=step-export-minimal \
  -DUSE_TCL=OFF -DUSE_TK=OFF -DBUILD_MODULE_Draw=OFF -DUSE_FREETYPE=OFF
cmake --build <build-dir> --config Release --target ezd2step -j8
cmake --build <build-dir> --config Release --target package_ezd2step_bundle
```

| Measurement | Result |
| --- | ---: |
| Configure | 25.92 s |
| Build | 386.77 s |
| Configure + build | 412.69 s |
| Incremental bundle packaging | 20.16 s |
| Installed `mac64/clang` tree | 38,568 KiB |
| Bundle directory | 38,576 KiB |
| Bundle archive | 13,970,726 bytes |
| CLI executable | 332,976 bytes |

Despite the profile's “no CAF” description, the target built and bundled 23 toolkits:

```text
TKBO TKBRep TKCAF TKCDF TKDE TKDESTEP TKG2d TKG3d TKGeomAlgo
TKGeomBase TKHLR TKLCAF TKMath TKMesh TKPrim TKService TKShHealing
TKTopAlgo TKV3d TKVCAF TKXCAF TKXSBase TKernel
```

## Curated `opencascade` proof

A standalone proof used the converter sources at the commit above, `find_package(OpenCASCADE CONFIG REQUIRED)`, package targets, `opencascade` 7.9.3 with default features disabled, and `nlohmann-json` 3.12.0. A dynamic arm64 triplet matched the existing shared-library product.

The proof manifest and triplet were:

```json
{
  "name": "ezd2step-dependency-proof",
  "version-string": "0",
  "dependencies": [
    { "name": "opencascade", "default-features": false },
    "nlohmann-json"
  ]
}
```

```cmake
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
```

Its CMake project created the CLI from `ezd2step.cxx`, `EzDesignJsonReader.cxx`, and `EzDesignToOCCTConverter.cxx`; the C API from `ezd_to_step.cxx` and the same converter sources; and `test_20260427_conversion` from the existing test and fixture. Each target linked the issue's 11 named OCCT targets:

```cmake
find_package(OpenCASCADE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(<target> PRIVATE nlohmann_json::nlohmann_json
  TKDESTEP TKXSBase TKDE TKBRep TKTopAlgo TKGeomAlgo TKGeomBase
  TKG3d TKG2d TKMath TKernel)
```

The exact workflow used a temporary `<proof-dir>` containing the manifest, triplet, and CMake project above, plus empty `<install-dir>` and `<build-dir>` directories:

```sh
./vcpkg/vcpkg install \
  --x-manifest-root=<proof-dir> \
  --x-install-root=<install-dir> \
  --triplet arm64-osx-dynamic \
  --overlay-triplets=<proof-dir>/triplets
cmake -S <proof-dir> -B <build-dir> \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_INSTALLED_DIR=<install-dir> \
  -DVCPKG_TARGET_TRIPLET=arm64-osx-dynamic \
  -DVCPKG_OVERLAY_TRIPLETS=<proof-dir>/triplets \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build <build-dir> --config Release -j8
ctest --test-dir <build-dir> --output-on-failure
```

The clean OCCT build (with downloads cached) took 897.93 s. The installed dependency tree was 233,600 KiB: 57,020 KiB Release libraries, 118,004 KiB debug libraries, 46,008 KiB headers, and 12,568 KiB shared data.

The curated port installed 49 OCCT toolkits. `otool -L` showed that the proof CLI directly linked the same 23-toolkit set as the vendored baseline above: the 11 named toolkits plus 12 exported transitive requirements.

The following 26 installed toolkits were not in that direct link set and are therefore demonstrated unused by this product:

```text
TKBin TKBinL TKBinTObj TKBinXCAF TKBool TKDECascade TKDEGLTF TKDEIGES
TKDEOBJ TKDEPLY TKDESTL TKDEVRML TKFeat TKFillet TKMeshVS TKOffset
TKOpenGl TKRWMesh TKStd TKStdL TKTObj TKXMesh TKXml TKXmlL TKXmlTObj
TKXmlXCAF
```

The package boundary itself works:

| Check | Result |
| --- | ---: |
| Configure | 4.84 s |
| Build CLI, C API, and test | 2.36 s |
| Conversion + STEP round-trip | Passed (3.00 s) |

However, `TKDESTEP`'s exported dependencies pull 12 toolkits beyond the starting ceiling into the CLI's direct link set. These are transitive requirements under the current package metadata and are not proven removable by this measurement. Disabling vcpkg default features does not change the OCCT module set and still builds OpenGL on macOS.

## Decision and blocker

The curated port proves that the standalone converter can use normal CMake package targets, but it does not meet the minimal build-surface criteria. A narrow registry-side OCCT override is justified to stop building the 26 demonstrated-unused toolkits. Any attempt to remove the 12 exported transitive toolkits requires a separate source/link check first.

Issue #10 should place the narrow OCCT override in the separate registry and
rerun the same macOS arm64 measurements plus Windows x64 CI. That optimization
is deliberately outside the repository split in issue #9.
