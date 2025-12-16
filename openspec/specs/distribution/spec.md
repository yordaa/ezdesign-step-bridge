# Distribution Capability

## Requirements

### Requirement: Redistributable Binary Bundles

The system SHALL provide redistributable binary bundles containing the `ezd2step` executable and required OCCT dynamic libraries for macOS and Windows platforms.

#### Scenario: macOS bundle structure
- **GIVEN** a macOS build of `ezd2step`
- **WHEN** the bundle is created
- **THEN** the bundle contains `ezd2step` executable
- **AND** the bundle contains all required OCCT `.dylib` libraries
- **AND** libraries are in the same directory as the executable
- **AND** the executable has rpath configured to locate bundled libraries

#### Scenario: Windows bundle structure
- **GIVEN** a Windows build of `ezd2step`
- **WHEN** the bundle is created
- **THEN** the bundle contains `ezd2step.exe` executable
- **AND** the bundle contains all required OCCT `.dll` libraries
- **AND** libraries are in the same directory as the executable
- **AND** Windows DLL search path locates libraries automatically

#### Scenario: Bundle versioning
- **GIVEN** a build of `ezd2step` with version `<major>.<minor>.<patch>`
- **WHEN** the bundle is packaged
- **THEN** the bundle filename includes version and platform: `ezd2step-<major>.<minor>.<patch>-<platform>.tar.gz` or `.zip`
- **AND** version information is accessible (via `--version` flag or metadata)

### Requirement: Dynamic Linking Configuration

The system SHALL use dynamic linking exclusively and configure runtime library paths to locate OCCT libraries.

#### Scenario: macOS rpath configuration
- **GIVEN** a macOS bundle with `ezd2step` and OCCT libraries
- **WHEN** `ezd2step` is executed from the bundle directory
- **THEN** the executable locates OCCT libraries via rpath (`@loader_path` or `@rpath`)
- **AND** no `DYLD_LIBRARY_PATH` environment variable is required
- **AND** libraries load successfully from the bundle directory

#### Scenario: Windows DLL search path
- **GIVEN** a Windows bundle with `ezd2step.exe` and OCCT DLLs
- **WHEN** `ezd2step.exe` is executed from the bundle directory
- **THEN** Windows locates DLLs in the executable directory (default search path)
- **AND** no `PATH` modification is required for DLLs
- **AND** libraries load successfully from the bundle directory

#### Scenario: Environment variable override
- **GIVEN** a bundle with `ezd2step` and OCCT libraries
- **WHEN** `OCCT_LIB_PATH` (or platform-specific variable) is set to a different directory
- **THEN** `ezd2step` searches the override path before bundled libraries
- **AND** libraries from the override path are loaded if present
- **AND** bundled libraries are used as fallback if override path lacks required libraries

### Requirement: LGPL Compliance Package

The system SHALL provide a source package containing OCCT source code, applied patches, and build instructions matching the shipped binaries.

#### Scenario: Source package contents
- **GIVEN** a binary bundle is created
- **WHEN** the source package is generated
- **THEN** the package includes OCCT source code reference (commit/tag or full source)
- **AND** the package documents any patches applied to OCCT
- **AND** the package includes build instructions matching the binary build process
- **AND** the package documents toolchain versions (compiler, CMake, etc.)
- **AND** the package includes OCCT version/commit information
- **AND** the package includes LGPL 2.1 license text

#### Scenario: Build instructions reproducibility
- **GIVEN** the source package with build instructions
- **WHEN** a user follows the build instructions
- **THEN** the resulting binaries match the shipped binaries (same OCCT version, same patches)
- **AND** the build process is reproducible with documented toolchain versions

### Requirement: Artifact Distribution

The system SHALL publish distribution artifacts via CI/CD with versioning and checksums.

#### Scenario: Artifact naming
- **GIVEN** version `<major>.<minor>.<patch>` and platform `<platform>`
- **WHEN** artifacts are published
- **THEN** binary bundles are named `ezd2step-<major>.<minor>.<patch>-<platform>.tar.gz` (macOS) or `.zip` (Windows)
- **AND** source package is named `ezd2step-<major>.<minor>.<patch>-source.tar.gz`
- **AND** checksums file is named `SHA256SUMS`

#### Scenario: Checksum generation
- **GIVEN** distribution artifacts are created
- **WHEN** checksums are generated
- **THEN** SHA256 checksums are computed for all artifacts
- **AND** checksums are published in `SHA256SUMS` file
- **AND** checksums enable integrity verification by downstream users

#### Scenario: CI/CD publishing
- **GIVEN** a build completes successfully
- **WHEN** artifacts are ready for distribution
- **THEN** artifacts are published to GitHub Releases (or equivalent)
- **AND** artifacts are attached to versioned release
- **AND** release notes include version information and changelog

### Requirement: Bundle Validation

The system SHALL provide smoke tests validating bundle functionality and library loading.

#### Scenario: Executable execution
- **GIVEN** a bundle is extracted
- **WHEN** `ezd2step --help` or `ezd2step --version` is executed
- **THEN** the command executes successfully (exit code 0)
- **AND** help text or version information is displayed

#### Scenario: Conversion functionality
- **GIVEN** a bundle and a valid sample `.ezd` file
- **WHEN** `ezd2step input.ezd output.step` is executed
- **THEN** conversion completes successfully (exit code 0)
- **AND** a valid STEP file is created
- **AND** the STEP file can be read back by OCCT

#### Scenario: Error handling
- **GIVEN** a bundle and an invalid input file
- **WHEN** `ezd2step invalid.ezd output.step` is executed
- **THEN** the command exits with non-zero exit code
- **AND** error messages are displayed on stderr

#### Scenario: Library loading
- **GIVEN** a bundle is extracted
- **WHEN** `ezd2step` is executed
- **THEN** all required OCCT libraries load successfully
- **AND** no library loading errors occur
- **AND** library paths are resolved correctly (rpath/DLL search)

