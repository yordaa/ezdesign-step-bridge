# Quick Fix Plan: Critical Gaps Before Implementation

## Overview
Address critical gaps in the codebase to align with `publish-json2step-bundles` proposal requirements.

## Critical Fixes Required

### 1. Tool Renaming: `json2step` → `ezd2step`

**Files to rename:**
- `tools/json2step/` → `tools/ezd2step/`
- `tools/json2step/json2step.cxx` → `tools/ezd2step/ezd2step.cxx`
- `tools/json2step/CMakeLists.txt`: Update `project(json2step)` → `project(ezd2step)`
- `tools/json2step/FILES`: Update first line `json2step.cxx` → `ezd2step.cxx`

**References to update:**
- `CMakeLists.txt` (line 1278-1279): `OCCT_ADD_SUBDIRECTORY ("tools/json2step")` → `OCCT_ADD_SUBDIRECTORY ("tools/ezd2step")`
- `tools/CMakeLists.txt` (line 376): Comment update
- `tools/json2step/CMakeLists.txt`: 
  - Line 1: `project(json2step)` → `project(ezd2step)`
  - Line 27: Comment update
  - Line 52: Comment update
- `tools/json2step/FILES`: Line 1: `json2step.cxx` → `ezd2step.cxx`
- `tools/json2step/test_basic_models.cxx`: 
  - Line 62: Function name `findJson2Step()` → `findEzd2Step()`
  - Line 74-77: Path references `json2step` → `ezd2step`
  - Line 89: Return value `"json2step"` → `"ezd2step"`
  - Line 96: Parameter name `json2stepPath` → `ezd2stepPath`
  - Line 102, 104, 108, 206, 209, 210, 211, 221, 230, 239: All references
- `tools/json2step/PROPRIETARY_SETUP.md`: Line 12: Update reference
- `tools/json2step/NOTICE`: Line 4: Update directory name in comment
- `tools/json2step/LICENSE`: Line 6: Update directory name in comment

**Additional updates:**
- Update usage message in `json2step.cxx`: Change `<input.json>` → `<input.ezd>` to match proposal
- Update example: `model.json` → `model.ezd`

**Estimated effort:** 30 minutes

---

### 2. Exit Code Contract Implementation

**Current state:** All errors return `1`

**Required mapping:**
- `0`: Success
- `1`: Invalid arguments (wrong argc, missing files)
- `2`: File I/O error (cannot read input, cannot write output)
- `3`: JSON parsing error (ReadFile fails, validation fails)
- `4`: Conversion error (ConvertBody fails, shape is null)
- `5`: STEP export error (Transfer fails, Write fails)

**File to modify:** `tools/json2step/json2step.cxx` (will be `ezd2step.cxx` after rename)

**Changes needed:**
```cpp
// Line 45-48: Invalid arguments
if (argc != 3) {
  printUsage(argv[0]);
  return 1;  // ✓ Already correct
}

// Line 60-67: JSON parsing error
if (!reader.ReadFile(...)) {
  // ...
  return 3;  // Change from 1 to 3
}

// Line 75-82: JSON validation error
if (!reader.Validate()) {
  // ...
  return 3;  // Change from 1 to 3
}

// Line 91-100: Conversion error
if (shape.IsNull()) {
  // ...
  return 4;  // Change from 1 to 4
}

// Line 116-119: STEP transfer error
if (status != IFSelect_RetDone) {
  // ...
  return 5;  // Change from 1 to 5
}

// Line 123-126: STEP write error
if (status != IFSelect_RetDone) {
  // ...
  return 5;  // Change from 1 to 5
}

// Add file I/O error handling:
// Check if input file exists before reading
// Check if output directory is writable
// Return 2 for file I/O errors
```

**Estimated effort:** 45 minutes (including file existence checks)

---

### 3. Add `--version` Flag

**File to modify:** `tools/json2step/json2step.cxx` (will be `ezd2step.cxx`)

**Changes needed:**

1. **Add version constants:**
```cpp
// After includes, before printUsage
#define EZD2STEP_VERSION_MAJOR 1
#define EZD2STEP_VERSION_MINOR 0
#define EZD2STEP_VERSION_PATCH 0

// Or read from CMake if available
#ifndef EZD2STEP_VERSION
#define EZD2STEP_VERSION "1.0.0"
#endif
```

2. **Add version printing function:**
```cpp
void printVersion()
{
  std::cout << "ezd2step version " << EZD2STEP_VERSION << std::endl;
  std::cout << "Copyright (c) 2025 Yang Song. All rights reserved." << std::endl;
}
```

3. **Update main() to handle --version:**
```cpp
int main(int argc, char* argv[])
{
  // Check for --version or --help flags
  if (argc == 2) {
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
      printVersion();
      return 0;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      printUsage(argv[0]);
      return 0;
    }
  }
  
  if (argc != 3) {
    printUsage(argv[0]);
    return 1;
  }
  // ... rest of main
}
```

4. **Update CMakeLists.txt to define version:**
```cmake
# In tools/ezd2step/CMakeLists.txt
project(ezd2step VERSION 1.0.0)

# Add version define
target_compile_definitions(${PROJECT_NAME} PRIVATE
  EZD2STEP_VERSION="${PROJECT_VERSION}"
)
```

**Estimated effort:** 30 minutes

---

## Implementation Order

1. **Rename tool first** (affects all other changes)
2. **Add --version flag** (simple addition)
3. **Implement exit code contract** (requires careful error categorization)

**Total estimated time:** ~2 hours

---

## Testing Checklist

After fixes:
- [ ] `ezd2step --version` prints version and exits with 0
- [ ] `ezd2step --help` prints usage and exits with 0
- [ ] `ezd2step` (no args) prints usage and exits with 1
- [ ] Invalid input file exits with 2 (file I/O error)
- [ ] Invalid JSON exits with 3 (JSON parsing error)
- [ ] Conversion failure exits with 4 (conversion error)
- [ ] STEP write failure exits with 5 (STEP export error)
- [ ] Successful conversion exits with 0

---

## Notes

- nlohmann/json is header-only, so no bundling needed (just document requirement)
- Environment variable override can be implemented later (not blocking)
- C API is optional and can be deferred

