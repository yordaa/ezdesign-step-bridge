# EZDesign Integration Prompt

## Context

The `ezd2step` converter has been fully implemented and is ready for integration into EZDesign. This document provides a comprehensive summary for the EZDesign development team to integrate the library.

## What Has Been Implemented

### 1. Command-Line Tool (`ezd2step`)
- **Location**: `tools/ezd2step/ezd2step.cxx`
- **Functionality**: Converts EZDesign JSON files (`.ezd`) to STEP format (ISO 10303-21)
- **Interface**: `ezd2step <input.ezd> <output.step>`
- **Exit Codes**:
  - `0`: Success
  - `1`: Invalid arguments
  - `2`: File I/O error
  - `3`: JSON parsing error
  - `4`: Geometry conversion error
  - `5`: STEP export error

### 2. C API Library (`libezd2step`)
- **Header**: `tools/ezd2step/ezd_to_step.h`
- **Implementation**: `tools/ezd2step/ezd_to_step.cxx`
- **Library Name**: `libezd2step.dylib` (macOS) / `ezd2step.dll` (Windows)
- **Function Signature**:
  ```c
  int ezd_to_step(const char* input_path, const char* output_path, 
                  const struct ezd_to_step_options* options);
  ```

### 3. Options Structure
```c
struct ezd_to_step_options {
  int verbose;           // 0 = quiet, 1 = progress, 2 = debug
  const char* log_file;  // NULL = no log file, or path to log file
};
```

### 4. Bundle Distribution
- **Binary Bundle**: `ezd2step-1.0.0-macos-arm64.tar.gz` (13MB)
  - Contains: `ezd2step` executable + all required OCCT libraries
  - Self-contained, no external dependencies
  - Libraries use `@loader_path` rpath (macOS) for automatic loading
  
- **Source Package**: `ezd2step-1.0.0-source.tar.gz` (27KB)
  - LGPL compliance package
  - Build instructions and OCCT source reference

## Integration Options

### Option 1: Use Pre-built Bundle (Recommended)
1. Extract the bundle: `tar -xzf ezd2step-1.0.0-macos-arm64.tar.gz`
2. Call the executable: `./ezd2step input.ezd output.step`
3. Check exit code for success/failure

**Pros**: No build required, self-contained, tested
**Cons**: Larger bundle size (~13MB)

### Option 2: Link Against C API Library
1. Include header: `#include "ezd_to_step.h"`
2. Link library: `-lezd2step` (or `ezd2step.lib` on Windows)
3. Call function:
   ```c
   struct ezd_to_step_options opts = {1, NULL};  // verbose=1, no log
   int result = ezd_to_step("input.ezd", "output.step", &opts);
   if (result != 0) {
       // Handle error based on exit code
   }
   ```

**Pros**: Smaller integration, programmatic control
**Cons**: Requires linking against OCCT libraries or using bundle's libraries

### Option 3: Build from Source
1. Follow instructions in `ezd2step-1.0.0-source.tar.gz`
2. Build OCCT as shared libraries
3. Build ezd2step and link into your application

**Pros**: Full control, can customize
**Cons**: Requires OCCT build setup

## Technical Details

### Library Dependencies
The C API library depends on:
- OCCT libraries (TKDESTEP, TKXSBase, TKDE, TKBRep, etc.)
- nlohmann/json (header-only, already included)

### Runtime Library Loading
- **macOS**: Uses `@loader_path` rpath - libraries must be in same directory as executable/library
- **Windows**: DLLs in same directory (default search path)
- **Override**: Set `DYLD_LIBRARY_PATH` (macOS) or `PATH` (Windows) to use custom OCCT libraries

### Error Handling
All errors return non-zero exit codes. Error messages are written to:
- `stderr` (if no log file specified)
- Log file (if `log_file` option is set)

### Thread Safety
The C API is **not thread-safe**. Use external synchronization if calling from multiple threads.

## Example Integration Code

### C/C++ Example
```c
#include "ezd_to_step.h"
#include <stdio.h>

int convert_ezd_to_step(const char* input, const char* output) {
    struct ezd_to_step_options opts = {
        .verbose = 1,        // Show progress messages
        .log_file = NULL     // No log file
    };
    
    int result = ezd_to_step(input, output, &opts);
    
    switch (result) {
        case 0:
            printf("Conversion successful\n");
            return 0;
        case 1:
            fprintf(stderr, "Invalid arguments\n");
            break;
        case 2:
            fprintf(stderr, "File I/O error\n");
            break;
        case 3:
            fprintf(stderr, "JSON parsing error\n");
            break;
        case 4:
            fprintf(stderr, "Geometry conversion error\n");
            break;
        case 5:
            fprintf(stderr, "STEP export error\n");
            break;
        default:
            fprintf(stderr, "Unknown error\n");
            break;
    }
    
    return result;
}
```

### Python Example (using ctypes)
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

## File Locations

### Source Files
- C API Header: `tools/ezd2step/ezd_to_step.h`
- C API Implementation: `tools/ezd2step/ezd_to_step.cxx`
- CLI Tool: `tools/ezd2step/ezd2step.cxx`
- Core Conversion: `tools/ezd2step/EzDesignToOCCTConverter.*`
- JSON Reader: `tools/ezd2step/EzDesignJsonReader.*`

### Documentation
- C API Docs: `tools/ezd2step/C_API_README.md`
- Distribution Guide: `tools/ezd2step/DISTRIBUTION_README.md`
- Build Instructions: Included in source package

### Build Artifacts
- Binary Bundle: `build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz`
- Source Package: `build/bundles/ezd2step-1.0.0-source.tar.gz`
- Shared Library: `build/mac64/clang/lib/libezd2step.dylib`

## Testing

### Smoke Tests
Run: `tools/ezd2step/test_bundle.sh <bundle_archive>`
- Validates bundle structure
- Tests executable functionality
- Verifies library dependencies

### Environment Override Test
Run: `tools/ezd2step/test_env_override.sh <bundle_directory>`
- Validates DYLD_LIBRARY_PATH override mechanism

## Next Steps for EZDesign Integration

1. **Choose Integration Method**: Decide between CLI tool, C API, or building from source
2. **Bundle Distribution**: Determine how to distribute the bundle with EZDesign
3. **Error Handling**: Implement error handling based on exit codes
4. **Testing**: Test with actual EZDesign `.ezd` files
5. **Documentation**: Update EZDesign docs with conversion workflow

## Key Points

- ✅ Fully functional and tested
- ✅ LGPL compliant (source package included)
- ✅ Self-contained bundles (no external dependencies)
- ✅ C API for programmatic integration
- ✅ Comprehensive error handling
- ✅ Cross-platform (macOS arm64, Windows x64 ready)
- ✅ Version: 1.0.0

## Questions to Consider

1. Should EZDesign bundle `ezd2step` with the application or download it separately?
2. Do you need the C API or is CLI tool sufficient?
3. What error handling strategy fits EZDesign's architecture?
4. Should conversion happen synchronously or asynchronously?
5. Do you need progress callbacks or is the current verbose option sufficient?

## Repository Information

- **Git Commit**: `ff13a9541f6545f63e3040ce94005d458a383ee1`
- **Branch**: `rename-json2step-to-ezd2step`
- **OCCT Version**: 7.9.1
- **Build System**: CMake with OCCT toolkit integration

---

**Ready for Integration**: The `ezd2step` converter is production-ready and can be integrated into EZDesign immediately.


