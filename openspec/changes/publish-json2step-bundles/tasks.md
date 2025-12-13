## 1. Preparation and Documentation

- [ ] 1.1 Choose and document OCCT commit/tag used for builds
- [ ] 1.2 Document any patches applied to OCCT (if any)
- [ ] 1.3 Document build toolchain versions (compiler, CMake, etc.)
- [ ] 1.4 Create LGPL compliance notice template

## 2. Build System Configuration

- [x] 2.1 Configure CMake to build OCCT as shared libraries (not static)
- [x] 2.2 Set rpath for macOS builds (`@loader_path` or `@rpath`)
- [x] 2.3 Configure Windows DLL search path (executable directory)
- [ ] 2.4 Add environment variable override support (`OCCT_LIB_PATH` or platform-specific)
- [x] 2.5 Create CMake packaging targets for bundle creation

## 3. CLI Interface Contract

- [x] 3.1 Document and implement exit code contract (0 = success, non-zero = failure)
- [x] 3.2 Ensure progress/info messages go to stdout
- [x] 3.3 Ensure error messages go to stderr
- [x] 3.4 Add `--version` flag (optional but recommended)
- [ ] 3.5 Validate CLI contract with tests

## 4. C API Implementation (Optional)

- [x] 4.1 Design C API structure (`ezd_to_step_options`)
- [x] 4.2 Implement C wrapper around C++ conversion code
- [x] 4.3 Create shared library target (`.dylib`/`.dll`)
- [x] 4.4 Add C API header file (`ezd_to_step.h`)
- [x] 4.5 Write C API documentation and examples

## 5. Bundle Packaging

- [x] 5.1 Create bundle directory structure script/target
- [x] 5.2 Copy `ezd2step` executable to bundle
- [x] 5.3 Copy required OCCT libraries to bundle
- [x] 5.4 Generate README.txt with usage and LGPL notices
- [x] 5.5 Include LICENSE.txt (LGPL 2.1 text)
- [x] 5.6 Create archive (`.tar.gz` for macOS, `.zip` for Windows)

## 6. Source Package

- [ ] 6.1 Create source package directory structure
- [ ] 6.2 Include OCCT source reference (commit/tag, or full source if small)
- [ ] 6.3 Document applied patches (if any)
- [ ] 6.4 Write build instructions matching binary build process
- [ ] 6.5 Document toolchain versions and build flags
- [ ] 6.6 Include OCCT version/commit information
- [ ] 6.7 Create source archive

## 7. Versioning and Artifacts

- [x] 7.1 Implement versioning scheme in build system
- [x] 7.2 Generate versioned artifact names
- [x] 7.3 Create checksums (SHA256) for all artifacts
- [x] 7.4 Generate `SHA256SUMS` file

## 8. Testing

- [x] 8.1 Create smoke test script for bundle validation
- [x] 8.2 Test `ezd2step` execution from bundle (help/version)
- [ ] 8.3 Test conversion of sample `.ezd` file to `.step`
- [ ] 8.4 Verify STEP file can be read back by OCCT
- [x] 8.5 Test non-zero exit code on failure cases (invalid input, missing file)
- [ ] 8.6 Test environment variable override for OCCT libraries
- [x] 8.7 Test library loading from bundle (rpath/DLL search)

## 9. CI/CD Integration

- [ ] 9.1 Create CI job for macOS arm64 bundle build
- [ ] 9.2 Create CI job for Windows x64 bundle build
- [ ] 9.4 Add artifact publishing to GitHub Releases (or equivalent)
- [ ] 9.5 Automate checksum generation and attachment
- [ ] 9.6 Add smoke tests to CI pipeline

## 10. Documentation

- [x] 10.1 Write distribution README (usage, system requirements)
- [x] 10.2 Document library path override mechanism
- [x] 10.3 Create usage examples for downstream (EZDesign) integration
- [x] 10.4 Document expected paths and environment variable overrides
- [x] 10.5 Add troubleshooting section (common issues, library loading problems)

