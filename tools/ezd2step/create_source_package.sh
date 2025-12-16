#!/bin/bash
#=======================================================================
# Script: Create LGPL compliance source package
# Purpose: Package OCCT source reference, patches, and build instructions
#=======================================================================

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SOURCE_PKG_DIR="${PROJECT_ROOT}/build/source_package"
VERSION="${1:-1.0.0}"

echo "Creating source package for ezd2step version $VERSION..."
echo ""

# Create source package directory
rm -rf "$SOURCE_PKG_DIR"
mkdir -p "$SOURCE_PKG_DIR"

# Get OCCT version
OCC_VERSION_MAJOR=7
OCC_VERSION_MINOR=9
OCC_VERSION_MAINTENANCE=1
OCC_VERSION="${OCC_VERSION_MAJOR}.${OCC_VERSION_MINOR}.${OCC_VERSION_MAINTENANCE}"

# Get git commit (if in git repo)
if [ -d "$PROJECT_ROOT/.git" ]; then
    GIT_COMMIT=$(cd "$PROJECT_ROOT" && git rev-parse HEAD)
    GIT_REMOTE=$(cd "$PROJECT_ROOT" && git remote get-url origin 2>/dev/null || echo "Not available")
else
    GIT_COMMIT="Not available (not a git repository)"
    GIT_REMOTE="Not available"
fi

# Create build information file
cat > "$SOURCE_PKG_DIR/BUILD_INFO.txt" <<EOF
ezd2step Source Package - Build Information
===========================================

Version: $VERSION
Build Date: $(date -u +"%Y-%m-%d %H:%M:%S UTC")

OCCT Version Information
------------------------
OCCT Version: $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE
OCCT Source: https://dev.opencascade.org/
OCCT License: LGPL 2.1

This package was built using OCCT $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE.
The OCCT source code is available from the official Open CASCADE Technology repository.

Repository Information
----------------------
Git Commit: $GIT_COMMIT
Repository: $GIT_REMOTE

Build Toolchain
---------------
CMake: $(cmake --version | head -1 | awk '{print $3}')
Compiler: $(clang --version 2>/dev/null | head -1 || gcc --version 2>/dev/null | head -1 || echo "Not available")
Platform: $(uname -s) $(uname -m)

Patches Applied
---------------
No patches have been applied to OCCT. This build uses standard OCCT $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE.

EOF

# Create build instructions
cat > "$SOURCE_PKG_DIR/BUILD_INSTRUCTIONS.md" <<'EOF'
# Build Instructions for ezd2step

This document provides instructions for building `ezd2step` from source, matching the binary build process.

## Prerequisites

### Required Software

- **CMake** 3.15 or later
- **C++ Compiler** with C++17 support:
  - macOS: Clang (Xcode Command Line Tools)
  - Linux: GCC 7+ or Clang 6+
  - Windows: MSVC 2019 or later
- **nlohmann/json** library
  - macOS: `brew install nlohmann-json`
  - Linux: `apt-get install nlohmann-json-dev` (or equivalent)
  - Windows: Use vcpkg or build from source

### OCCT Source Code

Download OCCT source code matching the version used in the binary build:

- **OCCT Version**: 7.9.1
- **Source**: https://dev.opencascade.org/
- **License**: LGPL 2.1

You can download OCCT from:
- Official repository: https://git.dev.opencascade.org/gitweb/?p=occt.git
- Or use the tagged release: https://git.dev.opencascade.org/gitweb/?p=occt.git;a=tags

## Build Process

### Step 1: Build OCCT

First, build OCCT as shared libraries:

```bash
# Configure OCCT build
cd /path/to/occt
cmake -B build -DBUILD_LIBRARY_TYPE=Shared

# Build OCCT
cmake --build build -j8

# Note: This will create shared libraries (.dylib on macOS, .dll on Windows)
```

### Step 2: Build ezd2step

```bash
# Configure ezd2step build
cd /path/to/ezdesign-step-bridge
cmake -B build -DBUILD_LIBRARY_TYPE=Shared

# Build ezd2step
cmake --build build --target ezd2step -j8

# Build C API library (optional)
cmake --build build --target ezd2step_capi -j8
```

### Step 3: Create Bundle (Optional)

To create a redistributable bundle:

```bash
cmake --build build --target package_ezd2step_bundle
```

The bundle will be created in `build/bundles/`.

## Build Configuration

### CMake Variables

Key CMake variables used in the build:

- `BUILD_LIBRARY_TYPE=Shared`: Build OCCT as shared libraries (required for LGPL compliance)
- `CMAKE_BUILD_TYPE=Release`: Release build (or Debug for development)

### Platform-Specific Notes

#### macOS

- Uses `@loader_path` rpath for library loading
- Libraries are built as `.dylib` files
- Requires Xcode Command Line Tools

#### Windows

- DLLs are placed in the same directory as the executable
- Uses default Windows DLL search path
- Requires Visual Studio or Build Tools

#### Linux

- Uses `RPATH` for library loading
- Libraries are built as `.so` files
- May require additional development packages

## Verification

After building, verify the executable:

```bash
# Test version
./build/mac64/clang/bin/ezd2step --version

# Test conversion (if you have a sample .ezd file)
./build/mac64/clang/bin/ezd2step input.ezd output.step
```

## Troubleshooting

### Issue: nlohmann/json not found

**Solution**: Install nlohmann/json:
- macOS: `brew install nlohmann-json`
- Linux: Use your package manager
- Windows: Use vcpkg: `vcpkg install nlohmann-json`

### Issue: OCCT libraries not found

**Solution**: Ensure OCCT is built and installed, or set library paths:
- macOS: `export DYLD_LIBRARY_PATH=/path/to/occt/lib:$DYLD_LIBRARY_PATH`
- Linux: `export LD_LIBRARY_PATH=/path/to/occt/lib:$LD_LIBRARY_PATH`
- Windows: Add OCCT lib directory to PATH

### Issue: Build fails with C++17 errors

**Solution**: Ensure your compiler supports C++17:
- Check compiler version
- Update CMakeLists.txt if needed to set C++17 standard

## Reproducing Binary Builds

To reproduce the exact binary build:

1. Use the exact OCCT version (7.9.1)
2. Use the same compiler version (see BUILD_INFO.txt)
3. Use the same CMake version
4. Build with `BUILD_LIBRARY_TYPE=Shared`
5. Use Release build type

The resulting binaries should match the distributed bundles.

EOF

# Create LGPL notice
cat > "$SOURCE_PKG_DIR/LGPL_NOTICE.txt" <<EOF
GNU LESSER GENERAL PUBLIC LICENSE
Version 2.1, February 1999

This software uses Open CASCADE Technology (OCCT), which is licensed under
the GNU Lesser General Public License version 2.1 (LGPL 2.1).

LGPL 2.1 Compliance
-------------------

This source package is provided to comply with the LGPL 2.1 license requirements
for distributing software that uses OCCT libraries.

What is Included:
- Build instructions matching the binary build process
- OCCT version information and source reference
- Toolchain versions and build configuration
- Instructions for obtaining OCCT source code

OCCT Source Code:
- Official repository: https://dev.opencascade.org/
- Version used: $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE
- License: LGPL 2.1

For the full text of the LGPL 2.1 license, please visit:
https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html

Important Notes:
- No patches have been applied to OCCT
- The binary build uses standard OCCT $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE
- You can replace OCCT libraries with your own builds (LGPL requirement)

EOF

# Copy relevant source files
echo "Copying source files..."
mkdir -p "$SOURCE_PKG_DIR/ezd2step"
cp -r "$PROJECT_ROOT/tools/ezd2step"/*.cxx "$PROJECT_ROOT/tools/ezd2step"/*.hxx "$PROJECT_ROOT/tools/ezd2step"/*.h "$PROJECT_ROOT/tools/ezd2step/CMakeLists.txt" "$PROJECT_ROOT/tools/ezd2step/FILES" "$PROJECT_ROOT/tools/ezd2step/EXTERNLIB" "$SOURCE_PKG_DIR/ezd2step/" 2>/dev/null || true

# Create README
cat > "$SOURCE_PKG_DIR/README.txt" <<EOF
ezd2step Source Package
======================

This package contains source code and build instructions for ezd2step version $VERSION.

Contents:
---------
- BUILD_INFO.txt          - Build information and toolchain versions
- BUILD_INSTRUCTIONS.md   - Detailed build instructions
- LGPL_NOTICE.txt         - LGPL 2.1 compliance notice
- ezd2step/              - ezd2step source code
- README.txt              - This file

Quick Start:
-----------
1. Read BUILD_INSTRUCTIONS.md for detailed build steps
2. Download OCCT source code (version $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE)
3. Build OCCT as shared libraries
4. Build ezd2step using the provided instructions

OCCT Source:
-----------
- Repository: https://dev.opencascade.org/
- Version: $OCC_VERSION_MAJOR.$OCC_VERSION_MINOR.$OCC_VERSION_MAINTENANCE
- License: LGPL 2.1

For questions or issues, see the main project repository.

EOF

# Create archive
ARCHIVE_NAME="ezd2step-${VERSION}-source.tar.gz"
ARCHIVE_PATH="${PROJECT_ROOT}/build/bundles/${ARCHIVE_NAME}"

echo ""
echo "Creating source archive..."
cd "$SOURCE_PKG_DIR/.."
tar -czf "$ARCHIVE_PATH" source_package

echo ""
echo "Source package created: $ARCHIVE_PATH"
echo ""
echo "Package contents:"
ls -lh "$SOURCE_PKG_DIR"

