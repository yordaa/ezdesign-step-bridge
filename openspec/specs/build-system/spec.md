## Purpose

The build system provides mechanisms for building OCCT with only a subset of modules enabled, reducing distribution size while maintaining functionality for specific use cases such as STEP file export/import, geometry operations, or data exchange.

## Requirements

### Requirement: Minimal Build Configuration
The build system SHALL support building OCCT with only a subset of modules enabled, reducing distribution size while maintaining functionality for specific use cases.

#### Scenario: Enable minimal build mode
- **WHEN** user sets `BUILD_MINIMAL_DISTRIBUTION=ON` in CMake configuration
- **THEN** build system only includes modules specified by the selected profile or custom module list
- **AND** all transitive dependencies are automatically included
- **AND** build completes successfully with reduced module set

#### Scenario: Select predefined profile
- **WHEN** user sets `BUILD_MINIMAL_PROFILE=step-export-minimal`
- **THEN** build system includes only modules required for shape-based STEP file import/export
- **AND** modules include: TKernel, TKMath, TKBRep, TKGeomBase, TKGeomAlgo, TKTopAlgo, TKDESTEP, and their dependencies
- **AND** CAF modules (TKCAF, TKXCAF) are excluded
- **AND** visualization and GUI modules are excluded
- **WHEN** user sets `BUILD_MINIMAL_PROFILE=step-export`
- **THEN** build system includes modules for full STEP file import/export with CAF support
- **AND** modules include: all from step-export-minimal plus TKCDF, TKCAF, TKXCAF
- **AND** both shape-based and document-based STEP APIs are available

#### Scenario: Custom module list
- **WHEN** user sets `BUILD_MINIMAL_DISTRIBUTION=ON` and `BUILD_MINIMAL_PROFILE=custom`
- **AND** user provides `BUILD_MINIMAL_CUSTOM_MODULES` list
- **THEN** build system includes only specified modules
- **AND** all required dependencies are automatically resolved and included
- **AND** build fails with clear error if required dependencies cannot be satisfied

### Requirement: Minimal Build Profiles
The build system SHALL provide predefined profiles for common use cases, each specifying a validated set of modules.

#### Scenario: STEP export minimal profile
- **WHEN** `BUILD_MINIMAL_PROFILE=step-export-minimal` is selected
- **THEN** profile includes modules: TKernel, TKMath, TKG2d, TKG3d, TKGeomBase, TKBRep, TKGeomAlgo, TKTopAlgo, TKShHealing, TKXSBase, TKDE, TKDESTEP
- **AND** includes all transitive dependencies of these modules
- **AND** excludes: CAF modules (TKCDF, TKCAF, TKXCAF), Visualization modules, Draw module
- **AND** resulting distribution is approximately 80-120MB (vs 500MB+ for full build)
- **AND** supports shape-based STEP operations only (STEPControl_Reader/Writer)
- **AND** does NOT support document-based operations (STEPCAFControl_Reader/Writer)

#### Scenario: STEP export profile
- **WHEN** `BUILD_MINIMAL_PROFILE=step-export` is selected
- **THEN** profile includes modules: TKernel, TKMath, TKG2d, TKG3d, TKGeomBase, TKBRep, TKGeomAlgo, TKTopAlgo, TKShHealing, TKXSBase, TKDE, TKDESTEP, TKCDF, TKCAF, TKXCAF
- **AND** includes all transitive dependencies of these modules
- **AND** excludes: Visualization modules, Draw module, ApplicationFramework modules (except CAF)
- **AND** resulting distribution is approximately 150-200MB (vs 500MB+ for full build)
- **AND** supports both shape-based (STEPControl_Reader/Writer) and document-based (STEPCAFControl_Reader/Writer) STEP operations
- **AND** supports STEP attributes: colors, layers, names, properties

#### Scenario: Geometry operations profile
- **WHEN** `BUILD_MINIMAL_PROFILE=geometry-only` is selected
- **THEN** profile includes modules: FoundationClasses, ModelingData, ModelingAlgorithms (core geometry and topology)
- **AND** excludes: DataExchange modules, Visualization modules, Draw module
- **AND** resulting distribution supports geometric modeling operations but not file I/O

#### Scenario: Data exchange profile
- **WHEN** `BUILD_MINIMAL_PROFILE=data-exchange` is selected
- **THEN** profile includes all DataExchange modules (STEP, IGES, STL, VRML, OBJ, PLY, etc.)
- **AND** includes required foundation and modeling modules
- **AND** excludes: Visualization modules, Draw module
- **AND** resulting distribution supports all CAD file formats but not visualization

### Requirement: Dependency Resolution
The build system SHALL automatically resolve and include all transitive dependencies for selected modules.

#### Scenario: Automatic dependency inclusion
- **WHEN** user selects module `TKDESTEP` in minimal build
- **THEN** build system automatically includes: TKXSBase, TKDE, TKernel, TKMath, TKBRep, TKGeomBase, TKGeomAlgo, TKTopAlgo, TKShHealing, TKG2d, TKG3d
- **AND** if document-based STEP is enabled (default), also includes: TKCDF, TKCAF, TKXCAF
- **AND** all their transitive dependencies are included
- **AND** build completes without missing symbol errors

#### Scenario: Dependency validation
- **WHEN** user specifies modules that have unsatisfied dependencies
- **THEN** build system detects missing dependencies
- **AND** CMake configuration fails with clear error message listing missing dependencies
- **AND** error message suggests required modules to add

### Requirement: CMake Integration
Minimal builds SHALL generate CMake configuration files compatible with `find_package(OpenCASCADE)`.

#### Scenario: Find package with minimal build
- **WHEN** external project uses `find_package(OpenCASCADE REQUIRED)`
- **AND** OpenCASCADE was built as minimal distribution
- **THEN** `find_package` succeeds
- **AND** only built modules are available as CMake targets
- **AND** linking against unavailable modules fails with clear error

#### Scenario: CMake target exports
- **WHEN** minimal build is installed
- **THEN** `OpenCASCADEConfig.cmake` is generated
- **AND** only built module targets are exported (e.g., `OpenCASCADE::TKDESTEP`)
- **AND** unavailable module targets are not exported
- **AND** target dependencies are correctly specified

### Requirement: Installation Structure
Minimal builds SHALL use the same installation layout as full builds, containing only files for built modules.

#### Scenario: Minimal installation layout
- **WHEN** minimal build is installed
- **THEN** installation directory contains: `bin/`, `lib/`, `include/opencascade/`
- **AND** `bin/` contains only DLLs/shared libraries for built modules
- **AND** `lib/` contains only import libraries/static libraries for built modules
- **AND** `include/opencascade/` contains only headers for built modules
- **AND** directory structure matches full build layout

#### Scenario: Platform-specific files
- **WHEN** minimal build is installed on Windows
- **THEN** `bin/` contains `.dll` files for built modules
- **AND** `lib/` contains `.lib` import libraries
- **WHEN** minimal build is installed on Linux/macOS
- **THEN** `lib/` contains `.so` or `.dylib` shared libraries
- **AND** library versioning and symlinks are preserved

### Requirement: Build Verification
The build system SHALL provide mechanisms to verify minimal builds are complete and functional.

#### Scenario: Build completeness check
- **WHEN** minimal build completes
- **THEN** build system verifies all required headers are present in include directory
- **AND** verifies all required libraries are present in lib directory
- **AND** verifies library dependencies are satisfied (no missing symbols)
- **AND** reports any issues during configuration phase

#### Scenario: Functionality test
- **WHEN** minimal build with `step-export` profile is created
- **THEN** basic STEP export functionality can be tested
- **AND** test verifies `STEPControl_Writer` can be instantiated and used
- **AND** test verifies no missing symbols at link time

#### Scenario: Runtime functionality test for STEP export
- **WHEN** minimal build with `step-export` profile is completed
- **AND** a test program is compiled and linked against the minimal build
- **THEN** test program can successfully include STEP-related headers (e.g., `STEPControl_Writer.hxx`)
- **AND** test program can instantiate `STEPControl_Writer` without runtime errors
- **AND** test program can create a simple shape and export it to STEP format
- **AND** exported STEP file is valid and can be read back
- **AND** no missing symbols or unresolved dependencies occur at runtime
- **AND** test program executes successfully and produces expected output

### Requirement: Module Build Selection
The build system SHALL support selective building of OCCT modules based on configuration options.

**Behavior**: 
- When `BUILD_MINIMAL_DISTRIBUTION=OFF` (default): All modules built as before
- When `BUILD_MINIMAL_DISTRIBUTION=ON`: Only modules specified by profile or custom list are built
- `BUILD_MODULE_*` options work in both modes, allowing further customization

#### Scenario: Backward compatibility
- **WHEN** `BUILD_MINIMAL_DISTRIBUTION` is not set (defaults to OFF)
- **THEN** build behavior is identical to current full build
- **AND** all existing build scripts continue to work
- **AND** no breaking changes to existing workflows

#### Scenario: Profile overrides module options
- **WHEN** `BUILD_MINIMAL_DISTRIBUTION=ON` and profile is selected
- **AND** user also sets `BUILD_MODULE_Visualization=ON`
- **THEN** profile module list takes precedence
- **AND** visualization modules are excluded if not in profile
- **AND** user receives warning about conflicting options
