# Standard Process for Updating vcpkg Port/Registry

This document describes the standard process for updating a vcpkg port and registry when both are maintained in the same repository. This process was developed from lessons learned during the ezd2step port updates.

## Critical Lessons Learned

1. **Git-tree hash must match**: The git-tree hash in `versions/e-/ezd2step.json` must exactly match the actual `ports/ezd2step` tree at that commit
2. **Version file must be regenerated**: Always regenerate the version file after port changes are committed
3. **Baseline must be updated**: The baseline in `versions/baseline.json` must match the new version
4. **Order matters**: Build → Release → Port Update → Registry Update
5. **Release must exist first**: GitHub release with binaries must exist before portfile references it

## Step-by-Step Process

### Scenario A: New Version Release (e.g., 1.0.0 → 1.0.1)

#### Step 1: Build the Binary Bundle

```bash
# Build the project
cd build
cmake ..
cmake --build . --target ezd2step -j8

# Create the bundle
cmake --build . --target package_ezd2step_bundle
```

#### Step 2: Generate Checksums

```bash
cd ..
./tools/ezd2step/create_checksums.sh

# Get the SHA256 hash for the portfile
cd build/bundles
NEW_SHA256=$(shasum -a 256 ezd2step-1.0.1-macos-arm64.tar.gz | awk '{print $1}')
echo "SHA256: $NEW_SHA256"  # Copy this for Step 4
```

#### Step 3: Create Git Tag and GitHub Release (BEFORE Port Update)

**Important**: The GitHub release must exist before the portfile references it, because the portfile downloads from `releases/tags/v${VERSION}`.

```bash
# Option A: Create tag first, then release (recommended)
git tag -a v1.0.1 -m "ezd2step v1.0.1"
git push origin v1.0.1

# Then create release with binaries
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

# Option B: Create release directly (creates tag automatically)
gh release create v1.0.1 \
  --title "ezd2step v1.0.1" \
  --notes "..." \
  build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS
```

#### Step 4: Update Port Files (AFTER Release Exists)

```bash
# 1. Update port version in vcpkg.json
# Edit ports/ezd2step/vcpkg.json: "version": "1.0.1"

# 2. Update portfile.cmake with SHA256 hash from Step 2
# Edit ports/ezd2step/portfile.cmake: 
#   set(ARCHIVE_SHA256 "NEW_SHA256_HASH_HERE")
```

#### Step 5: Commit Port Changes

```bash
git add ports/ezd2step/vcpkg.json
git add ports/ezd2step/portfile.cmake
git commit -m "Update ezd2step port to version 1.0.1"
git push origin main
```

#### Step 6: Regenerate Version File (CRITICAL)

**This step is critical** - it calculates the correct git-tree hash for the `ports/ezd2step` directory.

```bash
# After port changes are committed and pushed, regenerate the version file
vcpkg --x-builtin-ports-root=./ports \
      --x-builtin-registry-versions-dir=./versions \
      x-add-version ezd2step --all
```

#### Step 7: Verify Git-Tree Hash

```bash
# Verify the git-tree hash matches the actual port directory
git rev-parse HEAD:ports/ezd2step

# Compare with the hash in versions/e-/ezd2step.json
# They MUST match! If they don't, the version file is incorrect.
```

#### Step 8: Update Baseline

```bash
# Edit versions/baseline.json: Update "baseline": "1.0.1"
```

#### Step 9: Commit Registry Updates

```bash
git add versions/e-/ezd2step.json
git add versions/baseline.json
git commit -m "Update ezd2step baseline to version 1.0.1"
git push origin main
```

#### Step 10: Verify Final State

```bash
# Check that git-tree hash is correct
git rev-parse HEAD:ports/ezd2step
# Should match the hash in versions/e-/ezd2step.json for version 1.0.1

# Verify release exists and is downloadable
gh release view v1.0.1
curl -I https://github.com/yordaa/ezdesign-step-bridge/releases/download/v1.0.1/ezd2step-1.0.1-macos-arm64.tar.gz
```

---

### Scenario B: Fix Port Without Version Change (Bug Fix)

#### Step 1: Update Port Files

```bash
# Edit ports/ezd2step/portfile.cmake or other port files
```

#### Step 2: Commit Port Changes

```bash
git add ports/ezd2step/
git commit -m "fix: Fix portfile download logic"
git push origin main
```

#### Step 3: Regenerate Version File

```bash
# Even for bug fixes, regenerate to get correct git-tree hash
vcpkg --x-builtin-ports-root=./ports \
      --x-builtin-registry-versions-dir=./versions \
      x-add-version ezd2step --all
```

#### Step 4: Verify and Commit

```bash
# Verify git-tree hash matches
git rev-parse HEAD:ports/ezd2step

# Commit the updated version file
git add versions/e-/ezd2step.json
git commit -m "fix: Update version file with corrected git-tree hash"
git push origin main
```

---

## When to Create Git Tags

Create git tags when:
- ✅ You have a stable, tested release ready
- ✅ Binaries are built and verified
- ✅ You want to mark a specific commit as a release point
- ✅ **Before creating the GitHub release** (tag can be created by `gh release create`)

**Tag Format**: `v<major>.<minor>.<patch>` (e.g., `v1.0.1`)

```bash
# Create annotated tag (recommended)
git tag -a v1.0.1 -m "ezd2step v1.0.1"
git push origin v1.0.1

# Or let gh release create handle it
gh release create v1.0.1 ...  # Creates tag automatically
```

---

## When to Upload Binaries to GitHub Releases

Upload binaries when:
- ✅ The release is ready for distribution
- ✅ **BEFORE updating the vcpkg portfile** to reference that version
- ✅ The portfile downloads from `releases/tags/v${VERSION}`, so the release must exist first

**Upload Process**:

```bash
# Method 1: Upload during release creation (recommended)
gh release create v1.0.1 \
  --title "ezd2step v1.0.1" \
  --notes "..." \
  build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS

# Method 2: Upload to existing release
gh release upload v1.0.1 \
  build/bundles/ezd2step-1.0.1-windows-x64.zip

# Method 3: Create draft release, review, then publish
gh release create v1.0.1 \
  --draft \
  --title "ezd2step v1.0.1" \
  build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz

# Review on GitHub, then publish
gh release edit v1.0.1 --draft=false
```

---

## Complete Workflow Summary

```
1. Build bundle
   ↓
2. Generate checksums
   ↓
3. Create git tag
   ↓
4. Create GitHub release with binaries  ← MUST EXIST BEFORE PORT UPDATE
   ↓
5. Update port files (vcpkg.json, portfile.cmake)
   ↓
6. Commit port changes
   ↓
7. Regenerate version file (vcpkg x-add-version)
   ↓
8. Verify git-tree hash matches
   ↓
9. Update baseline
   ↓
10. Commit registry updates
   ↓
11. Verify final state
```

---

## Common Pitfalls and Fixes

| Problem | Symptom | Fix |
|---------|---------|-----|
| Release doesn't exist | Portfile download fails | Create GitHub release **before** updating portfile |
| Wrong git-tree hash | vcpkg fails to resolve version | Run `vcpkg x-add-version` after committing port changes |
| Baseline not updated | vcpkg uses old version | Update `versions/baseline.json` to match new version |
| SHA256 mismatch | vcpkg download fails | Ensure portfile SHA256 matches uploaded file |
| Tag created at wrong commit | Release points to wrong code | Create tag at commit with correct port files |
| Version file not regenerated | git-tree points to non-existent tree | Always regenerate after port changes |

---

## Verification Checklist

Before pushing, verify:

- [ ] Binary bundle built and tested
- [ ] SHA256 checksum generated and verified
- [ ] Git tag created (or will be created by `gh release create`)
- [ ] **GitHub release created with binaries uploaded** (must exist before port update)
- [ ] `ports/ezd2step/vcpkg.json` version matches release version
- [ ] `ports/ezd2step/portfile.cmake` SHA256 matches uploaded file
- [ ] `versions/baseline.json` baseline matches port version
- [ ] `versions/e-/ezd2step.json` has entry for the new version
- [ ] **Git-tree hash in version file matches**: `git rev-parse HEAD:ports/ezd2step`
- [ ] All changes committed and pushed

---

## Quick Reference Command

```bash
# Complete update workflow for new version

# 1. Build and prepare
cd build && cmake .. && cmake --build . --target ezd2step -j8
cmake --build . --target package_ezd2step_bundle
cd .. && ./tools/ezd2step/create_checksums.sh

# 2. Create release (creates tag automatically) - MUST BE FIRST
NEW_SHA256=$(shasum -a 256 build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz | awk '{print $1}')
gh release create v1.0.1 \
  --title "ezd2step v1.0.1" \
  --notes "..." \
  build/bundles/ezd2step-1.0.1-macos-arm64.tar.gz \
  build/bundles/SHA256SUMS

# 3. Update port (after release exists)
# Edit ports/ezd2step/vcpkg.json: "version": "1.0.1"
# Edit ports/ezd2step/portfile.cmake: ARCHIVE_SHA256="$NEW_SHA256"
git add ports/ezd2step/
git commit -m "Update port to version 1.0.1"
git push

# 4. Regenerate version file (CRITICAL)
vcpkg --x-builtin-ports-root=./ports \
      --x-builtin-registry-versions-dir=./versions \
      x-add-version ezd2step --all

# 5. Verify git-tree hash
git rev-parse HEAD:ports/ezd2step  # Must match version file

# 6. Update baseline and commit
# Edit versions/baseline.json: "baseline": "1.0.1"
git add versions/
git commit -m "Update baseline to version 1.0.1"
git push
```

---

## Key Takeaways

1. **Release First**: Always create the GitHub release with binaries **before** updating the portfile
2. **Regenerate Always**: Always run `vcpkg x-add-version` after committing port changes
3. **Verify Hash**: Always verify the git-tree hash matches `git rev-parse HEAD:ports/ezd2step`
4. **Update Baseline**: Always update `versions/baseline.json` to match the new version
5. **Order Matters**: Follow the workflow order strictly to avoid broken states

This process ensures the release exists before the portfile references it, and the git-tree hash correctly maps to the port directory state, preventing vcpkg resolution failures.
