# VCPKG Registry Setup - Complete ✅

## What Was Done

1. ✅ Created vcpkg registry structure:
   - `ports/ezd2step/vcpkg.json` - Port metadata
   - `ports/ezd2step/portfile.cmake` - Download and install logic
   - `ports/ezd2step/README.md` - Usage instructions
   - `versions/baseline.json` - Default baseline (v1.0.0)
   - `versions/e-/ezd2step.json` - Version file (placeholder)

2. ✅ Committed to git:
   - Commit 1: `af94dd8716` - Registry structure
   - Commit 2: `70faff8164` - Version file
   - **Current HEAD**: `70faff8164416d64ae69320b65340ec0ae8dfe1f`

## Important: Generate Proper Version File

The `versions/e-/ezd2step.json` file is currently a placeholder. You need to run vcpkg to generate the proper git-tree hash:

```bash
# From repository root, after pushing to GitHub:
vcpkg --x-builtin-ports-root=./ports \
      --x-builtin-registry-versions-dir=./versions \
      x-add-version ezd2step --all
```

This will update `versions/e-/ezd2step.json` with the correct git-tree hash.

## Next Steps

### 1. Push to GitHub
```bash
git push origin yang/step-mac
# or
git push origin main
```

### 2. Generate Version File (from EZDesign project or system vcpkg)
```bash
# Option A: From EZDesign project
cd /path/to/ezdesign
./vendor/vcpkg/vcpkg --x-builtin-ports-root=../ezdesign-step-bridge/ports \
                      --x-builtin-registry-versions-dir=../ezdesign-step-bridge/versions \
                      x-add-version ezd2step --all

# Option B: From system vcpkg
vcpkg --x-builtin-ports-root=./ports \
      --x-builtin-registry-versions-dir=./versions \
      x-add-version ezd2step --all
```

### 3. Commit the Updated Version File
```bash
git add versions/e-/ezd2step.json
git commit -m "fix: Update ezd2step version file with git-tree hash"
git push
```

### 4. Note Final Commit Hash for Baseline

After step 3, note the commit hash. This will be used in EZDesign's `vcpkg-configuration.json`:

```json
{
  "kind": "git",
  "repository": "https://github.com/yordaa/ezdesign-step-bridge",
  "baseline": "<final_commit_hash_after_version_file_update>",
  "packages": ["ezd2step"]
}
```

### 5. Test Installation

From EZDesign project:
```bash
vcpkg install ezd2step
```

Expected result:
- Downloads `ezd2step-1.0.0-macos-arm64.tar.gz` from GitHub Releases
- Extracts to `${VCPKG_INSTALLED_DIR}/tools/ezd2step/`
- If `EZDESIGN_RESOURCES_DIR` is set, also copies to `${EZDESIGN_RESOURCES_DIR}/ezd2step/`

## Current Status

- ✅ Registry structure created
- ✅ Files committed
- ⏳ Version file needs git-tree hash (run x-add-version)
- ⏳ Push to GitHub
- ⏳ Test installation from EZDesign

## Files Created

```
ports/ezd2step/
├── vcpkg.json          # Port metadata (v1.0.0)
├── portfile.cmake      # Download from GitHub Releases
└── README.md           # Usage instructions

versions/
├── baseline.json       # Default baseline
└── e-/
    └── ezd2step.json   # Version file (needs git-tree hash)
```

## SHA512 Hash

macOS arm64 bundle (v1.0.0):
```
f8b856f56cac6b09169d7170f47045b17e5663a7b137d5b8bc484297e4e84bbc038a1945e6a2a15eecb315b90f4b2a25e6420d1d9f91433b77b96cdb19c86062
```

