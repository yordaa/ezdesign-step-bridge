# Prompt for Updating EzDesign to Use Updated ezd2step

Copy this entire prompt into a new Cursor session for the EzDesign repository:

---

# Update EzDesign to Use Updated ezd2step with New Save Format Support

The `ezd2step` tool in the `ezdesign-step-bridge` repository has been updated to support the new EzDesign save format. I need to update EzDesign's vcpkg configuration to use the updated version.

## Context

The ezd2step tool has been updated with the following changes:
- Support for new entity format with `data` wrapper (all entity fields now under `data.fieldName`)
- Database structure change: `data.db.subd` → `data.db.bodies`
- Support for subdivision entity types (SubdivisionVertex, SubdivisionEdge, SubdivisionHalfEdge, SubdivisionFace, SubdivisionBody)
- Document version checking (requires >= 0.1.0)
- Made `surface_data` optional for SubdivisionFace entities

**Updated commit in ezdesign-step-bridge:** `541f2a14f270f35dc7ff18ffa32766f65fbc02c3`

## Task

1. **Locate vcpkg configuration file** in the EzDesign repository (likely `vcpkg-configuration.json` or similar)

2. **Update the baseline commit** for the `ezdesign-step-bridge` registry to point to the new commit:
   ```json
   {
     "kind": "git",
     "repository": "https://github.com/yordaa/ezdesign-step-bridge",
     "baseline": "541f2a14f270f35dc7ff18ffa32766f65fbc02c3",
     "packages": ["ezd2step"]
   }
   ```

3. **Verify the update** by:
   - Checking that vcpkg can resolve the ezd2step package
   - Testing that ezd2step can convert files in the new format
   - Ensuring integration code still works correctly

4. **Update any documentation** that references the ezd2step version or format requirements

## Files to Check

- `vcpkg-configuration.json` (or similar vcpkg config file)
- Any CMake files that reference ezd2step
- Integration code that calls ezd2step
- Documentation about file format compatibility

## Expected Outcome

After the update:
- EzDesign should be able to use ezd2step to convert files saved in the new format (version >= 0.1.0)
- The tool should handle both regular and subdivision entity types
- Conversion should work with the new `data.db.bodies` structure

## Reference

See `EZDESIGN_UPDATE_INSTRUCTIONS.md` in the ezdesign-step-bridge repository for detailed instructions.

---
