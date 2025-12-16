# OCCT - EzDesign to STEP Converter

This is a fork of [Open CASCADE Technology (OCCT)](https://github.com/Open-Cascade-SAS/OCCT) that provides a conversion bridge from the **ezdesign** JSON format to **STEP** files. The primary tool in this repository is `ezd2step`, which converts ezdesign geometry data into industry-standard STEP format.

## Overview

This repository extends OCCT with a specialized conversion tool (`ezd2step`) that:

- Reads ezdesign JSON files (`.ezd` format)
- Converts B-spline surfaces, curves, and topology to OCCT's geometric kernel
- Exports to STEP format (ISO 10303-21) for CAD interoperability
- Preserves topology sharing (shared edges between faces)
- Handles complex subdivision surface models

## License

This repository contains code under two different licenses:

### Proprietary Code (`tools/ezd2step/`)

All files in the `tools/ezd2step/` directory are **proprietary and confidential**. 
See `tools/ezd2step/LICENSE` for terms and conditions.

**Copyright (c) 2025 Yang Song. All rights reserved.**

### OCCT Code (Everything Else)

The rest of this repository is Open CASCADE Technology (OCCT), licensed under:

- **GNU Lesser General Public License version 2.1** (LGPL 2.1) with special exception defined in `OCCT_LGPL_EXCEPTION.txt`
- See `LICENSE_LGPL_21.txt` for complete license text
- Alternatively, OCCT may be used under Open CASCADE commercial license

**Note:** OCCT is provided on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND.

### Distribution Notice

When distributing binaries that link against OCCT libraries:
- OCCT portions remain under LGPL 2.1
- You must provide OCCT source code (or offer it)
- You must include LGPL license notices
- Your proprietary code in `tools/ezd2step/` can remain proprietary

## Building

### Prerequisites

- CMake 3.12 or later
- C++ compiler with C++17 support (Clang, GCC, or MSVC)
- [nlohmann/json](https://github.com/nlohmann/json) library
  - macOS: `brew install nlohmann-json`
  - Windows: Use vcpkg or build from source

### Build Instructions

```bash
# Configure build
cmake -B build

# Build ezd2step tool
cmake --build build --target ezd2step -j8

# Build test suite (optional)
cmake --build build --target test_basic_models -j8
```

The `ezd2step` executable will be located at:
- `build/mac64/clang/bin/ezd2step` (macOS)
- `build/linux64/gcc/bin/ezd2step` (Linux)
- `build/win64/vc14/bin/ezd2step.exe` (Windows)

## Usage

### Basic Conversion

```bash
ezd2step <input.ezd> <output.step>
```

Example:
```bash
ezd2step model.ezd model.step
```

### Test Suite

Run the test suite to verify conversion on sample models:

```bash
# Set path to ezd2step (if not in PATH)
export EZD2STEP_PATH=build/mac64/clang/bin/ezd2step

# Run tests
build/mac64/clang/bin/test_basic_models
```

The test suite validates:
- Single-face models
- Double-face models (with shared edges)
- Multi-face models (3+ faces)

Each test:
1. Converts the JSON file to STEP
2. Reads the STEP file back using OCCT
3. Validates face counts and topology

## Project Structure

```
tools/ezd2step/
├── ezd2step.cxx              # Main executable
├── EzDesignJsonReader.*      # JSON file parser
├── EzDesignToOCCTConverter.* # Core conversion logic
├── EzDesignTypes.hxx         # Data structure definitions
├── test_basic_models.cxx     # Test suite
└── CMakeLists.txt            # Build configuration
```

## Key Features

### Topology Preservation

The converter correctly handles shared edges between faces:
- Maps JSON `edge_id` to OCCT `TopoDS_Edge` objects
- Ensures a single `EDGE_CURVE` entity in STEP for geometrically identical edges
- Multiple `ORIENTED_EDGE` entities reference the same `EDGE_CURVE`

### Geometric Accuracy

- **Tolerance computation**: Vertex tolerances are computed based on actual distances between curve endpoints and vertex positions
- **Pcurve handling**: 2D parametric curves on surfaces are correctly associated with edges
- **Surface evaluation**: B-spline surfaces are properly constructed from control points and knot vectors

### Error Handling

The converter reports errors for:
- Invalid JSON structure
- Missing or invalid geometry data
- Edge creation failures
- Topology validation issues

Errors are collected and reported at the end of conversion.

## Limitations

- Rational B-spline surfaces (with weights) are currently treated as non-rational
- Only B-spline geometry is supported (no NURBS weights)
- Input must be valid ezdesign JSON format

## Development

### Adding New Features

The conversion pipeline:
1. `EzDesignJsonReader`: Parses JSON and builds in-memory data structures
2. `EzDesignToOCCTConverter`: Converts to OCCT topology (`TopoDS_Shape`)
3. `STEPControl_Writer`: Writes OCCT shapes to STEP format

To extend functionality:
- Modify `EzDesignToOCCTConverter` for new geometry types
- Update `EzDesignTypes.hxx` for new JSON structures
- Add validation in conversion methods

### Testing

Add new test cases to `test_basic_models.cxx` or create additional test executables following the same pattern.

## References

- [Open CASCADE Technology](https://dev.opencascade.org/) - Original OCCT project
- [STEP Format (ISO 10303-21)](https://www.iso.org/standard/63141.html) - CAD data exchange standard
- [OCCT Documentation](https://dev.opencascade.org/doc/overview) - OCCT API reference

## Version

Based on OCCT version defined in [`adm/cmake/version.cmake`](adm/cmake/version.cmake).
