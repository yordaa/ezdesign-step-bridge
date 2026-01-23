# How to Update ezd2step Binary on GitHub Releases

## Overview

The vcpkg port downloads pre-built bundles from GitHub Releases. When you update the source code, you need to:

1. Build a new bundle with the updated code
2. Create/update a GitHub release with the new bundle
3. Update the vcpkg portfile with the new SHA256 checksum

## How It Works

### Current Setup

- **vcpkg portfile**: `ports/ezd2step/portfile.cmake`
- **Downloads from**: GitHub Releases (tag `v${VERSION}`)
- **Expected file**: `ezd2step-${VERSION}-${PLATFORM}.tar.gz` (or `.zip` on Windows)
- **Verification**: SHA256 checksum (hardcoded in portfile)

### Current Release

- **Version**: v1.0.0
- **macOS arm64**: `ezd2step-1.0.0-macos-arm64.tar.gz`
- **SHA256**: `ed1f9b706ff3df5820bfeab04852d15a4384c3e565f0bd8b806f9267c52140e8`

## Step-by-Step: Update Binary for New Code Changes

### Option 1: Create New Version Release (Recommended)

This is the cleanest approach - creates a new version (e.g., v1.0.1).

#### Step 1: Build the Bundle

```bash
cd /Users/songyang/Codes/ezdesign-step-bridge

# Ensure you're on main with latest changes
git checkout main
git pull

# Build the project
cd build
cmake ..
cmake --build . --target ezd2step -j8

# Create the bundle
cmake --build . --target package_ezd2step_bundle
```

This creates: `build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz` (or new version)

#### Step 2: Generate Checksums

```bash
cd /Users/songyang/Codes/ezdesign-step-bridge
./tools/ezd2step/create_checksums.sh
```

This creates `build/bundles/SHA256SUMS` with the new SHA256 hash.

#### Step 3: Get the New SHA256 Hash

```bash
cd build/bundles
sha256sum ezd2step-1.0.0-macos-arm64.tar.gz
# Or on macOS:
shasum -a 256 ezd2step-1.0.0-macos-arm64.tar.gz
```

Copy the hash (first part, before the filename).

#### Step 4: Update Version (if creating new version)

If creating v1.0.1:

```bash
# Update version in portfile
# Edit ports/ezd2step/vcpkg.json: change "version": "1.0.0" to "1.0.1"
# Edit ports/ezd2step/portfile.cmake: update VERSION and ARCHIVE_SHA256
```

#### Step 5: Update Portfile with New SHA256

Edit `ports/ezd2step/portfile.cmake`:

```cmake
# For macOS arm64:
set(ARCHIVE_SHA256 "NEW_SHA256_HASH_HERE")
```

#### Step 6: Create GitHub Release

**Using GitHub CLI:**

```bash
# Create new version release
gh release create v1.0.1 \
  --title "ezd2step v1.0.1" \
  --notes "Update ezd2step for new EzDesign save format

## Changes
- Support for new entity format with data wrapper
- Database structure change: data.db.subd → data.db.bodies
- Support for subdivision entity types
- Document version checking (requires >= 0.1.0)

## Downloads
- macOS (arm64): ezd2step-1.0.1-macos-arm64.tar.gz" \
  build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS
```

**Or update existing v1.0.0 release:**

```bash
# Delete old asset (if needed)
gh release delete-asset v1.0.0 ezd2step-1.0.0-macos-arm64.tar.gz

# Upload new asset
gh release upload v1.0.0 \
  build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS \
  --clobber
```

#### Step 7: Update vcpkg Version File (if new version)

If you created a new version, regenerate the version file:

```bash
# This requires vcpkg to be installed
vcpkg --x-builtin-ports-root=./ports \
      --x-builtin-registry-versions-dir=./versions \
      x-add-version ezd2step --all
```

#### Step 8: Commit Changes

```bash
git add ports/ezd2step/portfile.cmake
git add ports/ezd2step/vcpkg.json  # if version changed
git add versions/e-/ezd2step.json  # if regenerated
git commit -m "Update ezd2step binary to v1.0.1 with new format support"
git push
```

### Option 2: Update Existing Release (Quick Fix)

If you want to keep the same version (v1.0.0) but update the binary:

1. Build new bundle (same as Step 1 above)
2. Get new SHA256 hash (same as Step 3 above)
3. Update portfile SHA256 (same as Step 5 above)
4. Upload new asset to existing release:

```bash
gh release upload v1.0.0 \
  build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS \
  --clobber
```

5. Commit portfile changes

**Note**: This approach updates the binary but keeps the same version number. Users will get the updated binary when they reinstall.

## Complete Workflow Example

Here's a complete example for updating to v1.0.1:

```bash
# 1. Build
cd build
cmake ..
cmake --build . --target ezd2step -j8
cmake --build . --target package_ezd2step_bundle

# 2. Generate checksums
cd ..
./tools/ezd2step/create_checksums.sh

# 3. Get SHA256
cd build/bundles
NEW_SHA256=$(shasum -a 256 ezd2step-1.0.1-macos-arm64.tar.gz | awk '{print $1}')
echo "New SHA256: $NEW_SHA256"

# 4. Update portfile (manually edit ports/ezd2step/portfile.cmake)
# Replace the SHA256 hash with $NEW_SHA256

# 5. Create release
cd ../..
gh release create v1.0.1 \
  --title "ezd2step v1.0.1" \
  --notes "Update for new EzDesign save format support" \
  build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS

# 6. Commit and push
git add ports/ezd2step/portfile.cmake
git commit -m "Update ezd2step to v1.0.1 with new format support"
git push
```

## Verification

After updating, verify the release:

```bash
# Check release exists
gh release view v1.0.1

# Verify asset is downloadable
curl -L -o /tmp/test.tar.gz \
  https://github.com/yordaa/ezdesign-step-bridge/releases/download/v1.0.1/ezd2step-1.0.1-macos-arm64.tar.gz

# Verify checksum
shasum -a 256 /tmp/test.tar.gz
# Should match the hash in portfile
```

## Important Notes

1. **Version Numbering**: Use semantic versioning (major.minor.patch)
   - Patch (1.0.1): Bug fixes, format compatibility updates
   - Minor (1.1.0): New features, backward compatible
   - Major (2.0.0): Breaking changes

2. **SHA256 Must Match**: The portfile SHA256 must exactly match the uploaded file, or vcpkg will fail

3. **Multiple Platforms**: If you have Windows bundles, update both:
   - macOS: `ezd2step-1.0.1-macos-arm64.tar.gz`
   - Windows: `ezd2step-1.0.1-windows-x64.zip`

4. **Testing**: Always test the bundle before releasing:
   ```bash
   tar -xzf build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz
   ./ezd2step-1.0.1-macos-arm64/ezd2step --version
   ```

## Troubleshooting

**Issue: vcpkg fails with "SHA256 mismatch"**
- Solution: Ensure the SHA256 in portfile exactly matches the uploaded file

**Issue: Release asset not found**
- Solution: Check the asset name matches exactly (case-sensitive)

**Issue: Can't upload to existing release**
- Solution: Use `--clobber` flag to replace existing assets
