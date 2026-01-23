# Instructions for Updating EzDesign to Use Updated ezd2step

## Summary

The `ezd2step` tool has been updated to support the new EzDesign save format (version >= 0.1.0). The changes include:

- Support for new entity format with `data` wrapper
- Database structure change: `data.db.subd` → `data.db.bodies`
- Support for subdivision entity types (SubdivisionVertex, SubdivisionEdge, etc.)
- Document version checking (requires >= 0.1.0)

**Commit:** `541f2a14f270f35dc7ff18ffa32766f65fbc02c3`

## Update Steps for EzDesign

### Option 1: Update vcpkg Baseline (Recommended)

If EzDesign uses this repository as a vcpkg registry, update the baseline commit in `vcpkg-configuration.json`:

```json
{
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/yordaa/ezdesign-step-bridge",
      "baseline": "541f2a14f270f35dc7ff18ffa32766f65fbc02c3",
      "packages": ["ezd2step"]
    }
  ]
}
```

**Note:** The vcpkg port definition hasn't changed (still v1.0.0), but updating the baseline ensures you're using the latest registry state. The actual ezd2step binary will be updated when a new release is created with the updated bundles.

### Option 2: Wait for New Release

If you're using pre-built bundles from GitHub Releases:

1. Wait for a new release (e.g., v1.0.1) to be created with updated bundles
2. The vcpkg port will automatically use the new release when available
3. No changes needed to `vcpkg-configuration.json` if using version pinning

### Option 3: Build from Source (If Applicable)

If EzDesign builds ezd2step from source instead of using pre-built bundles:

1. Update the baseline commit as shown in Option 1
2. Rebuild ezd2step to get the updated source code
3. The updated code will handle the new save format

## Testing

After updating, test with a file saved in the new format (version >= 0.1.0):

```bash
# Test conversion
ezd2step test_file.ezd output.step
```

The tool should now successfully parse files with:
- `data.db.bodies` structure (instead of `data.db.subd`)
- Entities with `data` wrapper (e.g., `{"id": 1, "type": "Body", "data": {"shells": [...]}}`)
- Subdivision entity types (SubdivisionVertex, SubdivisionEdge, etc.)

## Verification

To verify the update worked:

1. Check that ezd2step can parse new format files
2. Verify conversion produces valid STEP files
3. Test with files containing subdivision entities

## Rollback

If issues occur, you can rollback by setting the baseline to the previous commit:
- Previous commit: `068f0990343d23453c9020d9de54b578527a3a0e` (from versions/e-/ezd2step.json)

However, note that the old version will not support the new save format.
