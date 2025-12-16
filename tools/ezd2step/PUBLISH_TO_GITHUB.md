# Publishing ezd2step Artifacts to GitHub

This guide explains how to create distribution artifacts and publish them on GitHub Releases.

## Step 1: Create the Bundle

Build the bundle package:

```bash
cd build
cmake --build . --target package_ezd2step_bundle
```

This creates:
- `build/bundles/ezd2step-<version>-<platform>.tar.gz` (or `.zip` on Windows)
- `build/bundles/ezd2step-<version>-<platform/` (unpacked bundle directory)

## Step 2: Generate Checksums

Create SHA256 checksums for verification:

```bash
./tools/ezd2step/create_checksums.sh
```

This creates `build/bundles/SHA256SUMS` with checksums for all bundles.

## Step 3: Publish to GitHub Releases

### Method A: Using GitHub CLI (Recommended)

**Create a new release with artifacts:**

```bash
# Create release with bundle and checksums
gh release create v1.0.0 \
  --title "ezd2step v1.0.0" \
  --notes "Initial release of ezd2step - EZDesign to STEP converter

## Features
- Convert EZDesign JSON (.ezd) to STEP format
- Self-contained bundles with OCCT libraries
- C API for programmatic integration

## Downloads
- macOS (arm64): ezd2step-1.0.0-macos-arm64.tar.gz" \
  build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS
```

**Or create a draft release first (for review):**

```bash
gh release create v1.0.0 \
  --draft \
  --title "ezd2step v1.0.0" \
  --notes "Initial release" \
  build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS
```

**Add more assets to an existing release:**

```bash
gh release upload v1.0.0 build/bundles/ezd2step-1.0.0-windows-x64.zip
gh release upload v1.0.0 build/bundles/ezd2step-1.0.0-source.tar.gz
```

**Edit an existing release:**

```bash
gh release edit v1.0.0 --notes "Updated release notes"
```

### Method B: Using GitHub Web UI

1. Go to your repository on GitHub
2. Click **"Releases"** → **"Draft a new release"**
3. Fill in:
   - **Tag version**: `v1.0.0` (create new tag)
   - **Release title**: `ezd2step v1.0.0`
   - **Description**: Release notes
4. **Attach binaries**: Drag and drop files from `build/bundles/`
   - `ezd2step-1.0.0-macos-arm64.tar.gz`
   - `SHA256SUMS`
5. Click **"Publish release"**

### Method C: Using Git Tags + Manual Upload

**Create and push a tag:**

```bash
git tag -a v1.0.0 -m "ezd2step v1.0.0"
git push origin v1.0.0
```

Then use GitHub web UI to create a release from the tag and upload files.

## Step 4: Create Source Package (for LGPL Compliance)

For LGPL compliance, create a source package:

```bash
./tools/ezd2step/create_source_package.sh 1.0.0
```

This creates `build/source_package/` with:
- `BUILD_INFO.txt` - Build toolchain and OCCT version info
- `BUILD_INSTRUCTIONS.md` - Step-by-step build instructions
- Source code references

Then upload to the release:

```bash
# Create source archive (if not already created)
cd build/source_package
tar -czf ../bundles/ezd2step-1.0.0-source.tar.gz .

# Upload to release
gh release upload v1.0.0 build/bundles/ezd2step-1.0.0-source.tar.gz
```

## Complete Workflow Example

```bash
# 1. Build the project
cd build
cmake ..
cmake --build . --target ezd2step -j8

# 2. Create bundle
cmake --build . --target package_ezd2step_bundle

# 3. Generate checksums
cd ..
./tools/ezd2step/create_checksums.sh

# 4. Create source package (optional, for LGPL compliance)
./tools/ezd2step/create_source_package.sh 1.0.0
cd build/source_package
tar -czf ../bundles/ezd2step-1.0.0-source.tar.gz .
cd ../..

# 5. Create GitHub release
gh release create v1.0.0 \
  --title "ezd2step v1.0.0" \
  --notes "Initial release of ezd2step

## Downloads
- macOS (arm64): ezd2step-1.0.0-macos-arm64.tar.gz
- Source package: ezd2step-1.0.0-source.tar.gz

## Verification
Verify downloads using SHA256SUMS:
\`\`\`bash
sha256sum -c SHA256SUMS
\`\`\`" \
  build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz \
  build/bundles/ezd2step-1.0.0-source.tar.gz \
  build/bundles/SHA256SUMS
```

## Automated Publishing with GitHub Actions

To automate artifact creation and publishing, create a workflow file:

`.github/workflows/release.yml`:

```yaml
name: Create Release

on:
  release:
    types: [created]

jobs:
  build-and-publish:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [macos-latest, ubuntu-latest, windows-latest]
    
    steps:
    - uses: actions/checkout@v4
    
    - name: Build ezd2step
      run: |
        cmake -B build
        cmake --build build --target ezd2step -j8
    
    - name: Create bundle
      run: |
        cmake --build build --target package_ezd2step_bundle
    
    - name: Generate checksums
      run: |
        ./tools/ezd2step/create_checksums.sh
    
    - name: Upload artifacts
      uses: actions/upload-artifact@v4
      with:
        name: ezd2step-${{ matrix.os }}
        path: build/bundles/*.tar.gz
    
    - name: Upload to release
      uses: softprops/action-gh-release@v1
      with:
        files: build/bundles/*.tar.gz
        draft: false
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

## Verification

Users can verify downloaded artifacts:

```bash
# Download release files
wget https://github.com/your-repo/ezdesign-step-bridge/releases/download/v1.0.0/ezd2step-1.0.0-macos-arm64.tar.gz
wget https://github.com/your-repo/ezdesign-step-bridge/releases/download/v1.0.0/SHA256SUMS

# Verify checksum
sha256sum -c SHA256SUMS
```

## Best Practices

1. **Always include checksums** - Helps users verify download integrity
2. **Create source packages** - Required for LGPL compliance
3. **Use semantic versioning** - Follow `v<major>.<minor>.<patch>` format
4. **Write release notes** - Document changes, features, and known issues
5. **Test bundles before release** - Verify bundles work on target platforms
6. **Tag releases in git** - Creates permanent reference points

## Troubleshooting

**Issue: `gh release create` fails with authentication error**

Solution: Authenticate GitHub CLI:
```bash
gh auth login
```

**Issue: File too large for GitHub**

Solution: GitHub has a 2GB limit per file. For larger files, consider:
- Using Git LFS
- Splitting into multiple files
- Using external hosting (S3, etc.)

**Issue: Release already exists**

Solution: Either:
- Use a new version number
- Delete existing release: `gh release delete v1.0.0`
- Edit existing release: `gh release edit v1.0.0`

