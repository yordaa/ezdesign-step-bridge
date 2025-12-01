## Why

CAD applications often require only a subset of OCCT functionality (e.g., STEP file export/import) but must distribute the entire library, resulting in unnecessary bloat (500MB+ vs 150MB for minimal builds). Currently, there is no official mechanism to build a minimal OCCT distribution with only required modules, forcing developers to manually strip down installations or maintain custom build scripts.

## What Changes

- **ADDED**: CMake configuration option `BUILD_MINIMAL_DISTRIBUTION` to enable minimal build mode
- **ADDED**: Predefined minimal build profiles (e.g., `step-export`, `geometry-only`, `data-exchange`)
- **ADDED**: Automated dependency resolution for minimal builds (only required toolkits included)
- **ADDED**: Minimal distribution packaging script that creates `bin/`, `lib/`, `inc/` structure
- **ADDED**: CMake config file generation for minimal builds (enables `find_package(OpenCASCADE)`)
- **ADDED**: Documentation for creating custom minimal distributions
- **MODIFIED**: Build system to support selective module inclusion based on profile
- **MODIFIED**: Installation process to respect minimal build configuration

## Impact

- **Affected specs**: `build-system` capability
- **Affected code**: 
  - `CMakeLists.txt` (root and module-level)
  - `adm/cmake/occt_macros.cmake` (build logic)
  - `adm/cmake/occt_toolkit.cmake` (toolkit selection)
  - Build scripts in `adm/scripts/`
- **Breaking changes**: None (additive feature, opt-in via CMake option)
- **Dependencies**: No new external dependencies required
- **Platforms**: All supported platforms (Windows, Linux, macOS)


