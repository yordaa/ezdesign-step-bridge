# Minimal Build Configuration

OCCT supports building minimal distributions that include only a subset of modules, significantly reducing distribution size (typically 60-70% reduction).

## Quick Start

### Enable Minimal Build

```bash
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON -DBUILD_MINIMAL_PROFILE=step-export ..
```

### Available Profiles

- **step-export**: STEP file import/export only
  - Modules: FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
  - Use case: Applications that only need STEP file I/O

- **geometry-only**: Core geometry operations without data exchange or visualization
  - Modules: FoundationClasses, ModelingData, ModelingAlgorithms
  - Use case: Geometric modeling without file I/O

- **data-exchange**: All data exchange formats (STEP, IGES, STL, VRML, OBJ, PLY, glTF, etc.)
  - Modules: FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
  - Use case: Applications that need multiple CAD file formats

- **custom**: User-defined module list
  - Specify modules via `BUILD_MINIMAL_CUSTOM_MODULES`
  - Example: `-DBUILD_MINIMAL_CUSTOM_MODULES="FoundationClasses;ModelingData"`

## CMake Options

### BUILD_MINIMAL_DISTRIBUTION
Enable minimal build mode. When ON, only modules specified by the selected profile are built.

**Default**: OFF

### BUILD_MINIMAL_PROFILE
Select a predefined minimal build profile. Available values:
- `step-export`
- `geometry-only`
- `data-exchange`
- `custom`

**Default**: `step-export` (when BUILD_MINIMAL_DISTRIBUTION is ON)

### BUILD_MINIMAL_CUSTOM_MODULES
Semicolon-separated list of module names for custom profile. Only used when `BUILD_MINIMAL_PROFILE=custom`.

**Example**: `FoundationClasses;ModelingData;DataExchange`

## Dependency Resolution

Dependencies are automatically resolved. When you select a profile, all required toolkits and their transitive dependencies are included automatically.

For example, selecting `step-export` profile will automatically include:
- All toolkits from FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
- All transitive dependencies (e.g., TKXCAF is included because TKDESTEP depends on it)

## CMake Integration

Minimal builds generate full CMake configuration files compatible with `find_package(OpenCASCADE)`:

```cmake
find_package(OpenCASCADE REQUIRED)
# Only built modules are available
target_link_libraries(myapp OpenCASCADE::TKDESTEP)
```

If you try to link against a module that wasn't built, CMake will report an error during configuration.

## Installation

Minimal builds use the same installation layout as full builds:
- `bin/` - DLLs/shared libraries (only for built modules)
- `lib/` - Import libraries/static libraries (only for built modules)
- `include/opencascade/` - Headers (only for built modules)

The installation process automatically excludes modules that weren't built.

## Examples

### STEP Export Only

```bash
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON \
      -DBUILD_MINIMAL_PROFILE=step-export \
      ..
```

### Custom Minimal Build

```bash
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON \
      -DBUILD_MINIMAL_PROFILE=custom \
      -DBUILD_MINIMAL_CUSTOM_MODULES="FoundationClasses;ModelingData;ModelingAlgorithms" \
      ..
```

### Geometry Operations Only

```bash
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON \
      -DBUILD_MINIMAL_PROFILE=geometry-only \
      ..
```

## Size Comparison

Typical size reductions:
- **Full build**: ~500MB+
- **step-export profile**: ~150-200MB (60-70% reduction)
- **geometry-only profile**: ~100-150MB (70-80% reduction)
- **data-exchange profile**: ~200-250MB (50-60% reduction)

Actual sizes vary by platform and build configuration.

## Backward Compatibility

- Minimal builds are opt-in (BUILD_MINIMAL_DISTRIBUTION defaults to OFF)
- Full builds remain unchanged and work exactly as before
- Existing build scripts continue to work without modification
- No breaking changes to existing workflows

## Troubleshooting

### Module Not Found Error

If you get an error about a missing module when using `find_package`, ensure that module was included in your minimal build profile.

### Missing Symbols at Link Time

If you encounter missing symbols, check that all required dependencies are included. The build system should automatically include transitive dependencies, but if issues occur, try:
1. Using a more comprehensive profile (e.g., `data-exchange` instead of `step-export`)
2. Creating a custom profile with additional modules

### Profile Validation Errors

If you see errors about invalid modules in a profile, ensure module names match exactly (case-sensitive). Valid module names:
- FoundationClasses
- ModelingData
- ModelingAlgorithms
- ApplicationFramework
- DataExchange
- Visualization
- Draw
- DETools


