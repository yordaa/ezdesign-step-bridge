# ezd2step Distribution Package

## Overview

`ezd2step` is a command-line tool and C API library for converting EZDesign JSON files (`.ezd`) to STEP format (ISO 10303-21). This package includes pre-built binaries and required OCCT (Open CASCADE Technology) libraries for macOS and Windows.

**Version**: 1.0.0  
**Platform**: macOS (arm64) / Windows (x64)  
**License**: LGPL 2.1 (OCCT license)

## Quick Start

### 1. Extract the Bundle

```bash
# macOS
tar -xzf ezd2step-1.0.0-macos-arm64.tar.gz

# Windows
# Extract ezd2step-1.0.0-windows-x64.zip using your preferred tool
```

### 2. Run the Tool

```bash
cd ezd2step-1.0.0-macos-arm64
./ezd2step input.ezd output.step
```

That's it! The bundle is self-contained and requires no additional setup.

## System Requirements

### macOS
- macOS 11.0 (Big Sur) or later
- Apple Silicon (arm64) or Intel (x86_64) processor
- No additional dependencies required

### Windows
- Windows 10 or later
- x64 processor architecture
- No additional dependencies required

## Usage

### Command-Line Interface

**Basic conversion:**
```bash
ezd2step <input.ezd> <output.step>
```

**Examples:**
```bash
# Convert a model
./ezd2step model.ezd model.step

# Show version information
./ezd2step --version

# Show help
./ezd2step --help
```

**Exit Codes:**
- `0`: Success
- `1`: Invalid arguments
- `2`: File I/O error (input file not found, output directory not writable)
- `3`: JSON parsing error
- `4`: Geometry conversion error
- `5`: STEP export error

### C API

The bundle includes a C API library for programmatic integration. See `C_API_README.md` for detailed documentation.

**Quick example:**
```c
#include "ezd_to_step.h"

int result = ezd_to_step("input.ezd", "output.step", NULL);
if (result != 0) {
    // Handle error
}
```

## Library Path Override

For LGPL compliance, you can override the bundled OCCT libraries with your own versions.

### macOS

```bash
export DYLD_LIBRARY_PATH=/path/to/custom/occt/libs:$DYLD_LIBRARY_PATH
./ezd2step input.ezd output.step
```

The tool will search the override path before bundled libraries.

### Windows

```cmd
set PATH=C:\path\to\custom\occt\libs;%PATH%
ezd2step.exe input.ezd output.step
```

## Bundle Contents

```
ezd2step-1.0.0-<platform>/
├── ezd2step[.exe]          # Main executable
├── libTK*.dylib / *.dll    # OCCT libraries
├── README.txt              # Quick reference (this file)
├── LICENSE.txt             # LGPL 2.1 license text
└── C_API_README.md         # C API documentation (if included)
```

## Verification

### Verify Bundle Integrity

Checksums are provided in `SHA256SUMS`:

```bash
# macOS/Linux
sha256sum -c SHA256SUMS

# macOS (alternative)
shasum -a 256 -c SHA256SUMS
```

### Run Smoke Tests

A smoke test script is included to validate the bundle:

```bash
./test_bundle.sh ezd2step-1.0.0-macos-arm64.tar.gz
```

## Troubleshooting

### Issue: "Cannot open input file"

**Solution**: Check that the input file path is correct and the file exists.

```bash
ls -l input.ezd  # Verify file exists
./ezd2step input.ezd output.step
```

### Issue: "Output directory not writable"

**Solution**: Check write permissions on the output directory.

```bash
# macOS/Linux
chmod u+w /path/to/output/directory

# Or use a writable directory
./ezd2step input.ezd ~/output.step
```

### Issue: Library loading errors (macOS)

**Solution**: If you see `dyld: Library not loaded` errors:

1. Verify you're using the correct platform bundle (arm64 vs x86_64)
2. Check that all `.dylib` files are in the same directory as the executable
3. Try setting `DYLD_LIBRARY_PATH` to the bundle directory:

```bash
export DYLD_LIBRARY_PATH=$(pwd):$DYLD_LIBRARY_PATH
./ezd2step input.ezd output.step
```

### Issue: Library loading errors (Windows)

**Solution**: Ensure all `.dll` files are in the same directory as `ezd2step.exe`. Windows searches the executable directory automatically.

### Issue: "JSON parsing error"

**Solution**: Verify your `.ezd` file is valid EZDesign JSON format. Check the error messages for specific parsing issues.

### Issue: "Conversion error"

**Solution**: This usually indicates a problem with the geometry data in the JSON file. Check:
- Surface/curve data is valid
- Topology references are correct
- No missing or invalid references

## Integration Examples

### Python (using ctypes)

```python
import ctypes
import os

# Load library
lib = ctypes.CDLL('./libezd2step.dylib')  # macOS
# lib = ctypes.CDLL('./ezd2step.dll')     # Windows

# Define function signature
lib.ezd_to_step.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_void_p]
lib.ezd_to_step.restype = ctypes.c_int

# Call function
result = lib.ezd_to_step(
    b'input.ezd',
    b'output.step',
    None  # Use default options
)

if result == 0:
    print("Conversion successful!")
else:
    print(f"Conversion failed with exit code: {result}")
```

### Node.js (using ffi-napi)

```javascript
const ffi = require('ffi-napi');

const lib = ffi.Library('./libezd2step', {
  'ezd_to_step': ['int', ['string', 'string', 'pointer']]
});

const result = lib.ezd_to_step('input.ezd', 'output.step', null);
if (result === 0) {
  console.log('Conversion successful!');
} else {
  console.log(`Conversion failed with exit code: ${result}`);
}
```

## Source Code and Build Instructions

For LGPL compliance, source code and build instructions are provided in a separate source package:

- `ezd2step-1.0.0-source.tar.gz`

This package includes:
- OCCT source code reference
- Build instructions matching the binary build process
- Toolchain versions and build flags
- Applied patches (if any)

## Support

For issues, questions, or contributions:
- GitHub: https://github.com/your-repo/ezdesign-step-bridge
- Documentation: See `C_API_README.md` for C API details

## License

This software uses Open CASCADE Technology (OCCT), which is licensed under the GNU Lesser General Public License version 2.1 (LGPL 2.1).

See `LICENSE.txt` for the full license text.

**Important**: If you distribute software that uses `ezd2step`, you must comply with LGPL 2.1 requirements, including providing source code for OCCT libraries.

## Changelog

### Version 1.0.0
- Initial release
- macOS arm64 and Windows x64 bundles
- CLI tool with exit code contract
- C API for programmatic integration
- Self-contained bundles with OCCT libraries

