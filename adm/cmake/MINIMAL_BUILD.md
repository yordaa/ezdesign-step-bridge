# Minimal Build Configuration

OCCT supports building minimal distributions that include only a subset of modules, significantly reducing distribution size (typically 60-70% reduction).

## Quick Start

### Enable Minimal Build

```bash
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON -DBUILD_MINIMAL_PROFILE=step-export ..
```

### Available Profiles

- **step-export-minimal**: STEP file import/export (shape-based only, no CAF)
  - Modules: FoundationClasses, ModelingData, ModelingAlgorithms, DataExchange
  - Use case: Applications that only need basic STEP file I/O without document management
  - API: `STEPControl_Reader`/`STEPControl_Writer` only (no `STEPCAFControl_Reader`/`STEPCAFControl_Writer`)
  - Size: ~80-120MB (smallest STEP build)

- **step-export**: STEP file import/export with full CAF support
  - Modules: FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
  - Use case: Applications that need STEP file I/O with document management, colors, layers, names
  - API: Both `STEPControl_Reader`/`STEPControl_Writer` and `STEPCAFControl_Reader`/`STEPCAFControl_Writer`
  - Size: ~150-200MB (recommended for most use cases)

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
- `step-export-minimal` - STEP I/O without CAF (smallest)
- `step-export` - STEP I/O with CAF (recommended)
- `geometry-only` - Core geometry operations only
- `data-exchange` - All data exchange formats
- `custom` - User-defined module list

**Default**: `step-export` (when BUILD_MINIMAL_DISTRIBUTION is ON)

### BUILD_MINIMAL_CUSTOM_MODULES
Semicolon-separated list of module names for custom profile. Only used when `BUILD_MINIMAL_PROFILE=custom`.

**Example**: `FoundationClasses;ModelingData;DataExchange`

## Dependency Resolution

Dependencies are automatically resolved. When you select a profile, all required toolkits and their transitive dependencies are included automatically.

For example, selecting `step-export` profile will automatically include:
- All toolkits from FoundationClasses, ModelingData, ModelingAlgorithms, ApplicationFramework, DataExchange
- All transitive dependencies (e.g., TKXCAF is included because TKDESTEP depends on it)

Selecting `step-export-minimal` profile will include:
- All toolkits from FoundationClasses, ModelingData, ModelingAlgorithms, DataExchange
- All transitive dependencies (excluding CAF modules)

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

### STEP Export Only (Minimal, No CAF)

```bash
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON \
      -DBUILD_MINIMAL_PROFILE=step-export-minimal \
      ..
```

### STEP Export with CAF (Recommended)

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
- **step-export-minimal profile**: ~80-120MB (75-85% reduction)
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

## Runtime Functionality Test

A test program is provided to verify STEP export functionality works with minimal builds:

```bash
# After building minimal build, compile and run the test:
cd build-minimal-test
g++ -I../include/opencascade \
    -Lmac64/clang/lib \
    -lTKDESTEP -lTKSTEPBase -lTKSTEPAttr -lTKSTEP209 -lTKSTEP \
    -lTKXSBase -lTKDE -lTKTopAlgo -lTKGeomAlgo -lTKGeomBase \
    -lTKBRep -lTKG3d -lTKG2d -lTKMath -lTKernel \
    ../adm/cmake/test_minimal_step_export.cxx \
    -o test_minimal_step_export

# Run the test
./test_minimal_step_export test_output.step
```

The test program:
- Creates a simple box shape
- Instantiates STEPControl_Writer
- Exports the shape to STEP format
- Verifies the output file is created and valid

If the test passes, it confirms the minimal build STEP export functionality is working correctly.


