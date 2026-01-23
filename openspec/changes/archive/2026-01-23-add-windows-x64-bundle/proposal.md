## Why

The macOS arm64 bundle exists. Windows x64 bundle is needed for EZDesign Windows users.

## What Changes

- Implement ZIP creation in package_bundle.cmake (currently just prints a message)
- Build locally on Windows, run package_ezd2step_bundle target
- Upload ZIP to GitHub Releases v1.0.0
- Update portfile.cmake with SHA512 hash
- Update vcpkg.json supports field

## Impact

- Affected specs: `distribution`
- Affected code:
  - `tools/ezd2step/package_bundle.cmake` - add ZIP creation
  - `ports/ezd2step/portfile.cmake` - add SHA512 hash
  - `ports/ezd2step/vcpkg.json` - add Windows support
