# Update EzDesign to Use ezd2step v1.0.1 with New Save Format Support

## Context

The `ezd2step` tool in the `ezdesign-step-bridge` repository has been updated to version **v1.0.1** to support the new EzDesign save format. The new format was introduced in EzDesign commits:
- `3b2851b9bc28b0da612968294789e7c9a624505f`: Move database serialization to manifold library
- `0fb87de99e5ae923b021567f9eda4a10744596b0`: Add version check and use new data wrapper format

## What Changed in ezd2step v1.0.1

The ezd2step tool now supports:
- ✅ New entity format with `data` wrapper (all entity fields now under `data.fieldName`)
- ✅ Database structure change: `data.db.subd` → `data.db.bodies`
- ✅ Support for subdivision entity types (SubdivisionVertex, SubdivisionEdge, SubdivisionHalfEdge, SubdivisionFace, SubdivisionBody)
- ✅ Document version checking (requires >= 0.1.0)
- ✅ Made `surface_data` optional for SubdivisionFace entities

**Breaking Change**: This version breaks compatibility with old format files (version < 0.1.0). Old format files will be rejected with a clear error message.

## Task

Update EzDesign's vcpkg configuration to use ezd2step v1.0.1.

### Step 1: Locate vcpkg Configuration

Find the vcpkg configuration file in the EzDesign repository. It's likely:
- `vcpkg-configuration.json` (in project root or a config directory)
- Or a similar vcpkg config file

### Step 2: Update Registry Baseline

Update the baseline commit for the `ezdesign-step-bridge` registry to point to the latest commit:

```json
{
  "kind": "git",
  "repository": "https://github.com/yordaa/ezdesign-step-bridge",
  "baseline": "4337526696",
  "packages": ["ezd2step"]
}
```

**Note**: This will install ezd2step v1.0.1 which includes support for the new save format.

### Step 3: Verify the Update

1. **Check vcpkg can resolve the package:**
   ```bash
   vcpkg install ezd2step
   ```

2. **Verify the installed version:**
   ```bash
   # Check the installed ezd2step version
   ${VCPKG_INSTALLED_DIR}/tools/ezd2step/ezd2step --version
   ```

3. **Test with a new format file:**
   ```bash
   # Test conversion with a file saved in the new format (version >= 0.1.0)
   ${VCPKG_INSTALLED_DIR}/tools/ezd2step/ezd2step test_file.ezd output.step
   ```

### Step 4: Update Integration Code (if needed)

Check if any EzDesign code directly calls ezd2step and ensure it:
- Handles the new format correctly
- Works with the updated binary location
- Handles any error messages appropriately

### Step 5: Update Documentation

Update any documentation that references:
- ezd2step version number
- File format compatibility requirements
- Integration instructions

## Files to Check/Update

- `vcpkg-configuration.json` (or similar vcpkg config file) - **REQUIRED**
- Any CMake files that reference ezd2step
- Integration code that calls ezd2step (if any)
- Documentation about file format compatibility
- README or setup instructions

## Expected Outcome

After the update:
- ✅ EzDesign can use ezd2step v1.0.1 to convert files saved in the new format (version >= 0.1.0)
- ✅ The tool handles both regular and subdivision entity types
- ✅ Conversion works with the new `data.db.bodies` structure
- ✅ Old format files (version < 0.1.0) are properly rejected with clear error messages

## Reference Information

- **ezdesign-step-bridge repository**: https://github.com/yordaa/ezdesign-step-bridge
- **Latest commit (with v1.0.1)**: `4337526696`
- **Release**: https://github.com/yordaa/ezdesign-step-bridge/releases/tag/v1.0.1
- **Detailed instructions**: See `EZDESIGN_UPDATE_INSTRUCTIONS.md` in the ezdesign-step-bridge repository

## Testing Checklist

After updating, verify:
- [ ] vcpkg successfully installs ezd2step v1.0.1
- [ ] `ezd2step --version` shows correct version
- [ ] Conversion works with new format files (version >= 0.1.0)
- [ ] Old format files are rejected with appropriate error message
- [ ] Integration code (if any) still works correctly

## Troubleshooting

**Issue**: vcpkg can't find the package
- **Solution**: Verify the baseline commit hash is correct (`4337526696`)

**Issue**: Wrong version installed
- **Solution**: Clear vcpkg cache and reinstall: `vcpkg remove ezd2step && vcpkg install ezd2step`

**Issue**: Conversion fails with new format files
- **Solution**: Verify the file has version >= 0.1.0 in metadata and uses the new `data.db.bodies` structure
