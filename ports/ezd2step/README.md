# ezd2step vcpkg Port

This port downloads the pre-built `ezd2step` bundle from GitHub Releases.

## Usage

Add to `vcpkg-configuration.json`:
```json
{
  "kind": "git",
  "repository": "https://github.com/yordaa/ezdesign-step-bridge",
  "baseline": "<commit_hash>",
  "packages": ["ezd2step"]
}
```

Add to `vcpkg.json`:
```json
{
  "dependencies": ["ezd2step"]
}
```

## Installation

The port installs to `${VCPKG_INSTALLED_DIR}/tools/ezd2step/`.

If `EZDESIGN_RESOURCES_DIR` is set, it also copies to `${EZDESIGN_RESOURCES_DIR}/ezd2step/`.

## Platform Support

- macOS arm64 (current)
- Windows x64 (planned)

## Version

Current version: 1.0.0

