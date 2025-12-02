# STEP File Format Test Analysis

## Executive Summary

The OCCT codebase contains **~1,000+ STEP test cases** across multiple test grids. This analysis identifies which OCCT modules are required to run these tests and support STEP file export/import functionality.

## Test Infrastructure Overview

### Test Locations

1. **Main STEP Test Grids** (`tests/de/step_*`):
   - `step_1/`: 468 test cases
   - `step_2/`: 221 test cases
   - `step_3/`: 49 test cases
   - `step_4/`: 75 test cases
   - `step_5/`: 17 test cases
   - **Total: ~830 test cases**

2. **Bug-Specific Tests** (`tests/bugs/step/`):
   - 200+ bug-specific test cases
   - Tests regression fixes for specific STEP-related issues

3. **Data Exchange Wrapper Tests** (`tests/de_wrapper/step/`):
   - 6 test cases (A1-A6)
   - Tests unified DEWrapper API

4. **Additional Test Categories**:
   - `tests/de_mesh/step_read/` - Mesh reading tests
   - `tests/de_mesh/step_write/` - Mesh writing tests
   - `tests/metadata/step/` - Metadata handling tests

### Test Commands Used

The tests use two sets of commands:

1. **Document-based (CAF)**:
   - `ReadStep` - Reads STEP into a document
   - `WriteStep` - Writes document to STEP
   - Requires: CAF modules (TKCAF, TKXCAF, etc.)

2. **Shape-based (Non-CAF)**:
   - `stepread` - Reads STEP into a shape
   - `stepwrite` - Writes shape to STEP
   - Requires: Only core STEP modules

**Note**: The main test grids (`tests/de/step_*`) use document-based commands (`ReadStep`/`WriteStep`), which require CAF support.

## Module Dependency Analysis

### Core STEP Modules (Required for Basic STEP I/O)

#### Primary Toolkit: `TKDESTEP`
The main STEP data exchange toolkit containing:
- STEP file parsing and generation
- STEP entity definitions
- STEP-to-OCCT and OCCT-to-STEP translators

**Direct Dependencies** (from `src/TKDESTEP/EXTERNLIB`):
- `TKDE` - Data Exchange base
- `TKBRep` - Boundary Representation
- `TKernel` - Core kernel
- `TKMath` - Mathematics
- `TKXSBase` - XSTEP base infrastructure
- `TKTopAlgo` - Topology algorithms
- `TKG2d` - 2D geometry
- `TKG3d` - 3D geometry
- `TKGeomBase` - Geometry base
- `TKGeomAlgo` - Geometry algorithms
- `TKShHealing` - Shape healing
- `TKCAF` - Application Framework (for document-based operations)
- `TKCDF` - Common Data Framework
- `TKLCAF` - LCAF
- `TKXCAF` - XCAF (for document-based operations)

#### Control Modules: `STEPControl`
Located in `src/STEPControl/`, provides:
- `STEPControl_Reader` - Shape-based STEP reader
- `STEPControl_Writer` - Shape-based STEP writer

**Dependencies**:
- `XSControl_Reader` (from `TKXSBase`)
- `XSControl_WorkSession` (from `TKXSBase`)
- `TKDESTEP` (all STEP packages)

#### CAF Control Modules: `STEPCAFControl`
Located in `src/STEPCAFControl/`, provides:
- `STEPCAFControl_Reader` - Document-based STEP reader
- `STEPCAFControl_Writer` - Document-based STEP writer

**Dependencies**:
- `STEPControl_Reader`/`STEPControl_Writer` (base functionality)
- `TKXCAF` - Extended CAF
- `TKCAF` - Application Framework
- `TKCDF` - Common Data Framework

### Transitive Dependencies

#### `TKXSBase` Dependencies:
- `TKBRep`
- `TKernel`
- `TKMath`
- `TKG2d`
- `TKG3d`
- `TKTopAlgo`
- `TKGeomBase`
- `TKShHealing`

#### `TKDE` Dependencies:
- `TKernel`
- `TKMath`
- `TKBRep`

#### `TKBRep` Dependencies:
- `TKMath`
- `TKernel`
- `TKG2d`
- `TKG3d`
- `TKGeomBase`

#### `TKGeomBase` Dependencies:
- `TKernel`
- `TKMath`
- `TKG2d`
- `TKG3d`
- `CSF_TBB` (optional, for parallel processing)

#### `TKGeomAlgo` Dependencies:
- `TKernel`
- `TKMath`
- `TKG3d`
- `TKG2d`
- `TKGeomBase`
- `TKBRep`

#### `TKTopAlgo` Dependencies:
- `TKMath`
- `TKernel`
- `TKG2d`
- `TKG3d`
- `TKGeomBase`
- `TKBRep`
- `TKGeomAlgo`
- `CSF_TBB` (optional)

#### `TKShHealing` Dependencies:
- `TKBRep`
- `TKernel`
- `TKMath`
- `TKG2d`
- `TKTopAlgo`
- `TKG3d`
- `TKGeomBase`
- `TKGeomAlgo`
- `CSF_wsock32` (Windows-specific)

#### CAF Dependencies (for document-based operations):
- `TKCAF` → `TKCDF` → `TKernel`
- `TKXCAF` → `TKCAF` → `TKCDF` → `TKernel`
- `TKLCAF` → `TKCAF` → `TKCDF` → `TKernel`
- `TKDCAF` → `TKCAF` → `TKCDF` → `TKernel`
- `TKVCAF` → `TKXCAF` → `TKCAF` → `TKCDF` → `TKernel`

### Complete Module List for STEP Support

#### Minimal STEP (Shape-based, no CAF):
```
TKernel
TKMath
TKG2d
TKG3d
TKGeomBase
TKBRep
TKGeomAlgo
TKTopAlgo
TKShHealing
TKXSBase
TKDE
TKDESTEP
STEPControl (part of TKDESTEP)
```

#### Full STEP (Document-based, with CAF - required for tests):
```
# All modules from Minimal STEP, plus:
TKCDF
TKCAF
TKLCAF
TKXCAF
TKDCAF (optional, for document attributes)
TKVCAF (optional, for visualization attributes)
STEPCAFControl (part of TKDESTEP)
```

## Analysis of Colleague's Stripped-Down Version

The colleague's `OpenCASCADE-7.9_with_DataEx` includes:

**Core Modules**:
- `TKernel`, `TKMath`, `TKG2d`, `TKG3d`
- `TKGeomBase`, `TKGeomAlgo`
- `TKBRep`, `TKTopAlgo`, `TKShHealing`
- `TKXSBase`, `TKDE`, `TKDESTEP`

**CAF Modules** (for document-based operations):
- `TKCDF`, `TKCAF`, `TKLCAF`, `TKXCAF`, `TKVCAF`, `TKDCAF`

**Additional Data Exchange Modules**:
- `TKDEIGES`, `TKDEOBJ`, `TKDEPLY`, `TKDEVRML`, `TKDEGLTF`, `TKDESTL`

**Other Modules** (likely for shape operations):
- `TKBO`, `TKBool`, `TKFeat`, `TKFillet`, `TKOffset`, `TKPrim`
- `TKMesh`, `TKHLR`, `TKV3d`
- `TKService`, `TKStd`, `TKStdL`, `TKTObj`, `TKTopTest`
- `TKDECascade`, `TKExpress`

**Conclusion**: The colleague included CAF modules, indicating they're using document-based STEP operations (similar to the test infrastructure).

## Recommendations for Minimal Build Profile

### Profile 1: `step-export-minimal` (Shape-based only)
**Use Case**: Basic STEP import/export without document management

**Modules**:
```
TKernel
TKMath
TKG2d
TKG3d
TKGeomBase
TKBRep
TKGeomAlgo
TKTopAlgo
TKShHealing
TKXSBase
TKDE
TKDESTEP
```

**Excludes**:
- All CAF modules (TKCAF, TKXCAF, etc.)
- Visualization modules
- Development tools

**API Available**:
- `STEPControl_Reader`
- `STEPControl_Writer`
- Direct shape I/O

### Profile 2: `step-export` (Document-based, recommended)
**Use Case**: Full STEP support with document management, colors, layers, names

**Modules**: All from `step-export-minimal`, plus:
```
TKCDF
TKCAF
TKLCAF
TKXCAF
```

**API Available**:
- `STEPControl_Reader`/`STEPControl_Writer` (shape-based)
- `STEPCAFControl_Reader`/`STEPCAFControl_Writer` (document-based)
- Full attribute support (colors, layers, names, properties)

**Note**: This profile matches what the test infrastructure requires and what the colleague's stripped version provides.

## Test Compatibility

### Tests Requiring CAF (Majority):
- `tests/de/step_*` - All use `ReadStep`/`WriteStep` (document-based)
- `tests/de_wrapper/step/` - Use `ReadStep` (document-based)
- `tests/metadata/step/` - Require document structure

### Tests That Could Work Without CAF:
- Some shape-based tests using `stepread`/`stepwrite` directly
- However, the test infrastructure (`tests/de/begin` and `tests/de/end`) loads CAF modules by default

**Recommendation**: For test compatibility, include CAF modules in the `step-export` profile.

## Size Impact Analysis

### Estimated Module Sizes (approximate):
- Core geometry/math: ~50-100 MB
- TKDESTEP: ~20-30 MB
- CAF modules: ~10-20 MB
- **Total minimal STEP**: ~80-150 MB (vs. full OCCT ~500+ MB)

### Comparison:
- **Full OCCT**: ~500+ MB
- **Colleague's stripped version**: ~200-300 MB (includes multiple DE formats)
- **Minimal STEP-only**: ~80-150 MB

## Implementation Notes

1. **Dependency Resolution**: The build system must automatically include all transitive dependencies when `TKDESTEP` is selected.

2. **CAF Optionality**: Consider making CAF modules optional, with a flag like `BUILD_STEP_WITH_CAF` (default: ON for compatibility).

3. **Test Infrastructure**: The test system loads CAF by default. For minimal builds, either:
   - Include CAF (recommended for compatibility)
   - Or modify tests to support shape-based operations

4. **Resource Files**: STEP operations require resource files (e.g., `CSF_STEPDefaults`). These must be included in the distribution.

## Conclusion

For a minimal STEP export/import build that:
- Supports the existing test infrastructure
- Provides full STEP functionality (colors, layers, names)
- Matches the colleague's approach

**Recommended Profile**: `step-export` with CAF support

**Core Modules** (15 toolkits):
1. TKernel
2. TKMath
3. TKG2d
4. TKG3d
5. TKGeomBase
6. TKBRep
7. TKGeomAlgo
8. TKTopAlgo
9. TKShHealing
10. TKXSBase
11. TKDE
12. TKDESTEP
13. TKCDF
14. TKCAF
15. TKXCAF

This provides ~80-90% size reduction compared to full OCCT while maintaining full STEP functionality and test compatibility.

