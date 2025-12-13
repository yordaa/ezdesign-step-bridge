## Why

Users need redistributable binary bundles of the `ezd2step` converter for EZDesign integration. Currently, users must build OCCT and `ezd2step` from source, which requires significant build infrastructure and expertise. Providing pre-built, LGPL-compliant bundles enables downstream integration while maintaining compliance with OCCT's LGPL 2.1 license requirements.

## What Changes

- **ADDED**: Redistributable binary bundles for macOS and Windows containing `ezd2step` executable and required OCCT dynamic libraries
- **ADDED**: Versioned artifact naming and checksums for distribution integrity
- **ADDED**: LGPL compliance package including OCCT source code, applied patches, and build instructions matching shipped binaries
- **ADDED**: C API entrypoint (`ezd_to_step`) in shared library form for programmatic integration
- **ADDED**: Runtime library path configuration (rpath on macOS, DLL search path on Windows) with environment variable override support
- **ADDED**: Distribution artifacts published via CI/CD (GitHub Releases or equivalent)
- **ADDED**: Smoke tests validating bundle functionality and library loading
- **MODIFIED**: CLI interface contract formalized (exit codes, stdout/stderr usage)

## Impact

- **Affected specs**: 
  - `distribution` capability (new)
  - `json-reader` capability (modified to add C API)
- **Affected code**:
  - Build system: CMake packaging and rpath/DLL configuration
  - `tools/ezd2step/`: C API wrapper addition
  - CI/CD: Artifact generation and publishing workflows
  - Documentation: LGPL compliance notices and build instructions
- **Breaking changes**: None (additive feature)
- **Dependencies**: 
  - Existing OCCT build system
  - CI/CD infrastructure (GitHub Actions or equivalent)
  - Code signing tools (optional, for macOS/Windows)
- **Platforms**: macOS (arm64) and Windows (x64)

