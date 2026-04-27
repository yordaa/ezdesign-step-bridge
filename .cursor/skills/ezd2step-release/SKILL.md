---
name: ezd2step-release
description: Use when releasing ezd2step, pushing vX.Y.Z tags, creating GitHub release assets, or updating the ezd2step vcpkg port and registry.
---

# ezd2step Release

## Core Rule

Release assets must exist before the vcpkg port references a new version. The
vcpkg registry git-tree must be generated from the committed `ports/ezd2step`
tree, not hand-written.

Always read `VCPKG_PORT_UPDATE_PROCESS.md` before the vcpkg bump phase and
apply its critical lessons: release first, regenerate version metadata, verify
git-tree, update baseline, then commit registry updates.

## Preflight

1. Confirm the target version with the user, for example `1.0.2` and tag
   `v1.0.2`.
2. Check `git status -sb`. If unrelated user changes exist, do not touch them.
3. Sync `main` with `origin/main`.
4. Verify the tag and release do not already exist:
   - `git tag --list vX.Y.Z`
   - `gh release view vX.Y.Z`
5. Verify `.github/workflows/release-ezd2step.yml` supports tag-push release
   automation. It must build both bundles, smoke-test them, create or update
   the GitHub Release, upload source/checksums, and not require a pre-existing
   release. If this is not true, stop and fix the workflow before releasing.
6. Verify `tools/ezd2step/CMakeLists.txt` says
   `project(ezd2step VERSION X.Y.Z)`. If not, make a version-only commit on
   `main` or open/merge a PR if branch protection blocks direct push.

## Release Workflow

Run these in order:

1. Create and push the annotated tag from the verified `main` commit:
   ```bash
   git tag -a vX.Y.Z -m "ezd2step vX.Y.Z"
   git push origin vX.Y.Z
   ```
2. Watch the release workflow:
   ```bash
   gh run list --workflow release-ezd2step.yml --limit 5
   gh run watch <run-id> --exit-status
   ```
3. If CI fails, stop. Diagnose and fix the failure. Do not bump vcpkg.
4. Verify the non-draft, non-prerelease release exists and has all assets:
   - `ezd2step-X.Y.Z-macos-arm64.tar.gz`
   - `ezd2step-X.Y.Z-windows-x64.zip`
   - `ezd2step-X.Y.Z-source.tar.gz`
   - `SHA256SUMS`
5. Verify release asset availability with `gh release view vX.Y.Z` and direct
   download/API checks if needed.

## vcpkg Port And Registry Bump

Only start after the GitHub release assets are visible.

1. Read `VCPKG_PORT_UPDATE_PROCESS.md`.
2. Update `ports/ezd2step/vcpkg.json` to `"version": "X.Y.Z"`.
3. Check `ports/ezd2step/portfile.cmake`.
   - Current portfile resolves release assets dynamically and has no hardcoded
     SHA256, so no per-release change is needed.
   - If a future portfile reintroduces `ARCHIVE_SHA256`, update it from the
     uploaded artifact checksum before continuing.
4. Commit only the port changes:
   ```bash
   git add ports/ezd2step/vcpkg.json ports/ezd2step/portfile.cmake
   git commit -m "Update ezd2step port to version X.Y.Z"
   ```
5. Regenerate version metadata from the committed port tree:
   ```bash
   vcpkg --x-builtin-ports-root=./ports \
         --x-builtin-registry-versions-dir=./versions \
         x-add-version ezd2step --all
   ```
   If `vcpkg` is not on PATH, use an existing `./vcpkg/vcpkg` binary or
   bootstrap vcpkg locally. Do not commit the `vcpkg/` checkout.
6. Update `versions/baseline.json` so `ezd2step.baseline` is `X.Y.Z` and
   `port-version` is `0`.
7. Verify the git-tree:
   ```bash
   git_tree=$(git rev-parse HEAD:ports/ezd2step)
   ```
   Confirm it exactly matches the `git-tree` for version `X.Y.Z` in
   `versions/e-/ezd2step.json`.
8. Commit registry changes:
   ```bash
   git add versions/e-/ezd2step.json versions/baseline.json
   git commit -m "Update ezd2step baseline to version X.Y.Z"
   ```
9. Push the commits to `main`. If branch protection blocks direct push, open a
   PR with the two commits and stop for merge approval.

## Final Verification

Before reporting success, verify:

- `gh release view vX.Y.Z` shows all four release assets.
- `ports/ezd2step/vcpkg.json` is `X.Y.Z`.
- `versions/baseline.json` is `X.Y.Z`.
- `versions/e-/ezd2step.json` contains `X.Y.Z`.
- `git rev-parse HEAD:ports/ezd2step` matches the new registry `git-tree`.
- `git status -sb` has no unexpected tracked changes.

## Stop Conditions

Stop and ask before proceeding if:

- The target tag or release already exists.
- The release workflow does not support tag-push release creation.
- CI fails or release assets are missing.
- `x-add-version` changes unexpected files.
- The git-tree verification does not match.
- Pushing to `main` requires bypassing branch protection or force-pushing.
