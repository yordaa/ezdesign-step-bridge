# Proprietary License Setup

This directory has been configured for proprietary licensing. Here's what was done:

## Files Updated

All source files in this directory have been updated with proprietary copyright headers:

- `EzDesignTypes.hxx`
- `EzDesignJsonReader.hxx` / `EzDesignJsonReader.cxx`
- `EzDesignToOCCTConverter.hxx` / `EzDesignToOCCTConverter.cxx`
- `json2step.cxx`
- `test_basic_models.cxx`
- `test_step_roundtrip.cxx`

## New Files Created

1. **LICENSE** - Proprietary license terms for this directory
2. **NOTICE** - Notice explaining dual-license situation with OCCT

## Next Steps

1. **Replace `[Your Name/Company]`** in all files:
   - Search and replace in all `.hxx`, `.cxx` files
   - Update `LICENSE` file
   - Update `NOTICE` file
   - Update `README.md` (root level)

2. **Review LICENSE file** - Customize the proprietary license terms as needed

3. **If distributing binaries**:
   - Ensure OCCT source code is available (or offer it)
   - Include LGPL license notices for OCCT portions
   - Your proprietary code can remain closed-source

## Legal Compliance

- ✅ Your code is now marked as proprietary
- ✅ OCCT code remains LGPL (unchanged)
- ✅ Dual-license situation is clearly documented
- ⚠️ When distributing, you must comply with LGPL for OCCT portions

## Important Notes

- **Private use**: No restrictions - use internally as you wish
- **Distribution**: Must comply with LGPL for OCCT libraries
- **Source code**: OCCT source must be provided/offered when distributing binaries
- **License notices**: Must include LGPL notices for OCCT portions

