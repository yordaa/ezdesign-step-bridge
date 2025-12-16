# Design: Redistributable ezd2step Bundles

## Context

The `ezd2step` converter tool needs to be distributed as pre-built binaries for EZDesign integration. The distribution must comply with LGPL 2.1 requirements (OCCT license) while providing a simple deployment model for downstream users.

## Goals

1. **Redistributable Binaries**: Provide macOS and Windows bundles with `ezd2step` executable and required OCCT libraries
2. **LGPL Compliance**: Include OCCT source, patches, and build instructions matching shipped binaries
3. **Simple Deployment**: Users can extract and run without complex setup
4. **Library Flexibility**: Support environment variable overrides for OCCT library paths (enables relinking/replacement per LGPL)
5. **Versioning**: Clear versioning scheme for artifacts and compatibility tracking

## Non-Goals

- Static linking (violates LGPL spirit; users must be able to replace OCCT libraries)
- Code signing/DRM (adds barriers to LGPL compliance)
- Linux bundles (focus on macOS and Windows first)
- Installation packages (simple archive extraction preferred)
- macOS x86_64 bundles (focus on macOS arm64 and Windows x64)

## Architecture Decisions

### 1. Dynamic Linking Only

**Decision**: Use dynamic linking exclusively; no static OCCT libraries in bundles.

**Rationale**:
- LGPL 2.1 requires users to be able to relink/replace OCCT libraries
- Static linking would require commercial OCCT license or violate LGPL
- Dynamic linking enables library replacement without recompiling `ezd2step`
- Standard practice for LGPL-licensed libraries

**Implementation**:
- Build OCCT as shared libraries (`.dylib` on macOS, `.dll` on Windows)
- Bundle required OCCT libraries alongside `ezd2step` executable
- Configure rpath (macOS) or DLL search path (Windows) to locate bundled libraries
- Support environment variable override (`OCCT_LIB_PATH` or platform-specific vars)

### 2. Bundle Structure

**Decision**: Flat structure with executable and libraries in same directory.

**Rationale**:
- Simplest deployment model (extract and run)
- Works with rpath/DLL search path configuration
- Easy to override via environment variables
- No complex installation scripts needed

**Structure**:
```
ezd2step-<version>-<platform>/
├── ezd2step[.exe]          # Main executable
├── libTK*.dylib / *.dll     # OCCT libraries
├── README.txt               # Usage and LGPL notices
└── LICENSE.txt              # LGPL 2.1 text
```

**Alternative Considered**: Nested `bin/` and `lib/` directories
- **Rejected**: More complex rpath configuration, harder to override

### 3. Runtime Library Path Configuration

**Decision**: Use rpath (macOS) and DLL search path (Windows) with environment variable override.

**Rationale**:
- rpath enables self-contained bundles (no `DYLD_LIBRARY_PATH` required)
- Environment variable override enables users to replace OCCT libraries (LGPL requirement)
- Standard practice for redistributable binaries

**Implementation**:
- **macOS**: Set `@rpath` or `@loader_path` in executable, bundle libraries in same directory
- **Windows**: Place DLLs in same directory as executable (default search path)
- **Override**: Check `OCCT_LIB_PATH` (or `DYLD_LIBRARY_PATH`/`PATH`) before bundled paths

**Library Loading Priority**:
1. Environment variable override (if set)
2. Bundled libraries (rpath/executable directory)
3. System paths (fallback)

### 4. C API Entrypoint

**Decision**: Add optional C API (`ezd_to_step`) as shared library wrapper, keeping CLI as primary deliverable.

**Rationale**:
- Enables future programmatic integration (e.g., from other languages)
- C API is language-agnostic (can be called from Python, Node.js, etc.)
- Optional feature (CLI remains primary interface)
- Simple wrapper around existing C++ code

**API Design**:
```c
struct ezd_to_step_options {
    int verbose;           // 0 = quiet, 1 = progress, 2 = debug
    const char* log_file;  // Optional log file path
};

int ezd_to_step(const char* input_path, const char* output_path, 
                const struct ezd_to_step_options* options);
```

**Return Codes**:
- `0`: Success
- `1`: Invalid arguments
- `2`: File I/O error
- `3`: JSON parsing error
- `4`: Conversion error
- `5`: STEP export error

### 5. LGPL Compliance Package

**Decision**: Provide separate source package with OCCT source, patches, and build instructions.

**Rationale**:
- LGPL 2.1 requires providing source code for linked libraries
- Patches document any modifications to OCCT
- Build instructions enable users to reproduce binaries
- Separate package keeps binary bundle small

**Package Contents**:
- OCCT source code (Git commit/tag reference)
- List of applied patches (if any)
- Build instructions (CMake commands, toolchain versions)
- Toolchain notes (compiler versions, build flags)
- LGPL 2.1 license text
- OCCT version/commit information

**Distribution**:
- Separate archive: `ezd2step-<version>-source.tar.gz`
- Or included in binary bundle as `source/` directory (larger but simpler)

### 6. Versioning Scheme

**Decision**: Use semantic versioning with platform suffix: `ezd2step-<major>.<minor>.<patch>-<platform>`

**Rationale**:
- Clear version identification
- Platform suffix prevents confusion
- Semantic versioning aligns with common practices

**Examples**:
- `ezd2step-1.0.0-macos-arm64.tar.gz`
- `ezd2step-1.0.0-windows-x64.zip`
- `ezd2step-1.0.0-source.tar.gz`

**Version Components**:
- **Major**: Breaking API changes (unlikely for CLI)
- **Minor**: New features, backward-compatible changes
- **Patch**: Bug fixes, minor improvements

### 7. Distribution Channel

**Decision**: GitHub Releases (or equivalent CI/CD artifact publishing).

**Rationale**:
- Standard distribution mechanism for open-source projects
- Automatic checksums via GitHub
- Version tagging and release notes
- Accessible to downstream users

**Artifacts per Release**:
- Binary bundles (macOS arm64, Windows x64)
- Source package
- Checksums file (`SHA256SUMS`)
- Release notes (changelog)

### 8. Testing Strategy

**Decision**: Smoke tests validating bundle functionality and library loading.

**Rationale**:
- Ensures bundles work out-of-the-box
- Validates rpath/DLL configuration
- Catches library dependency issues early

**Test Cases**:
1. Extract bundle and run `ezd2step --version` (if implemented) or `ezd2step --help`
2. Convert sample `.ezd` file to `.step`
3. Verify STEP file can be read by OCCT
4. Test environment variable override (replace OCCT library with dummy and verify override works)
5. Verify non-zero exit code on failure cases

## Risks / Trade-offs

### Risk: Library Path Conflicts
- **Mitigation**: Clear documentation of library loading priority; test override mechanism

### Risk: OCCT Version Mismatch
- **Mitigation**: Document exact OCCT version/commit in source package; version binaries

### Risk: Platform-Specific Issues
- **Mitigation**: Test on clean VMs; document system requirements

### Trade-off: Bundle Size vs. Simplicity
- **Decision**: Prefer larger bundles (include all OCCT libraries) over complex dependency resolution
- **Rationale**: Simpler deployment, fewer support issues

## Migration Plan

1. **Phase 1**: Build and package macOS bundle (arm64)
2. **Phase 2**: Add Windows bundle (x64)
4. **Phase 4**: Add C API wrapper (optional, can be deferred)
5. **Phase 5**: Publish via CI/CD

## Open Questions

- Should we include nlohmann/json in bundle or require system installation?
  - **Decision**: Bundle if static linking possible, otherwise document requirement
- Code signing for macOS/Windows?
  - **Decision**: Optional; not required for LGPL compliance but improves user trust
- Support for Linux bundles?
  - **Decision**: Defer to future; focus on macOS/Windows first

