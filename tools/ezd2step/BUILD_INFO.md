# ezd2step Build Information

## OCCT Version

- **OCCT Version**: 7.9.1
- **OCCT Commit**: See main repository (this is an OCCT-based project)
- **OCCT Source**: https://dev.opencascade.org/
- **OCCT License**: LGPL 2.1

## Build Toolchain

- **CMake**: 3.30.5
- **Compiler**: Apple clang version 17.0.0 (clang-1700.4.4.1)
- **Build Type**: Shared libraries (dynamic linking)
- **Platform**: macOS arm64

## Build Configuration

- **BUILD_LIBRARY_TYPE**: Shared
- **RPATH Configuration**: `@loader_path` (macOS)
- **Library Location**: Same directory as executable

## Patches Applied

No patches applied to OCCT. Using standard OCCT 7.9.1 build.

## Build Instructions

1. Configure CMake:
   ```bash
   cmake -B build -DBUILD_LIBRARY_TYPE=Shared
   ```

2. Build:
   ```bash
   cmake --build build
   ```

3. Create bundle:
   ```bash
   cmake --build build --target package_ezd2step_bundle
   ```

## Bundle Contents

- `ezd2step` executable
- OCCT dynamic libraries (`.dylib` files)
- `README.txt` - Usage instructions
- `LICENSE.txt` - LGPL 2.1 license information

## Version Information

- **ezd2step Version**: 1.0.0
- **Build Date**: $(date)
- **Git Commit**: 53742d6aaddb7115ca6633e56e732b830daeb4b5

