## MODIFIED Requirements

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

#### Scenario: Windows x64 bundle creation
- **GIVEN** a Windows x64 build of ezd2step
- **WHEN** the package_ezd2step_bundle target is run
- **THEN** ezd2step.exe and required OCCT DLLs are bundled
- **AND** a ZIP archive is created as `ezd2step-<version>-windows-x64.zip`

