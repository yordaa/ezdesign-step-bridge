# Integration Guide: Using Minimal STEP Export in Your Project

This guide explains how to build OCCT with minimal STEP export capability and integrate it into your own C++ project.

## Table of Contents

1. [Building OCCT with Minimal STEP Export](#building-occt)
2. [Integrating into Your CMake Project](#cmake-integration)
3. [Using STEP Export in Your Code](#using-in-code)
4. [Troubleshooting](#troubleshooting)

---

## Building OCCT with Minimal STEP Export

### Step 1: Clone and Configure OCCT

```bash
# Navigate to where you want OCCT
cd ~/Codes
git clone <occt-repo-url> OCCT
cd OCCT

# Create build directory
mkdir build-minimal
cd build-minimal

# Configure with minimal STEP export profile
cmake -DBUILD_MINIMAL_DISTRIBUTION=ON \
      -DBUILD_MINIMAL_PROFILE=step-export-minimal \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=~/Codes/OCCT/install-minimal \
      ..
```

**Profile Options:**
- `step-export-minimal`: Shape-based STEP I/O only (~80-120MB, no CAF)
- `step-export`: Full STEP with CAF support (~150-200MB, recommended)

### Step 2: Build and Install

```bash
# Build (this will take some time)
cmake --build . -j$(sysctl -n hw.ncpu)

# Install to the prefix specified above
cmake --install .
```

After installation, you'll have:
```
~/Codes/OCCT/install-minimal/
├── bin/          # Shared libraries (.dylib on macOS, .dll on Windows, .so on Linux)
├── lib/          # Import libraries
├── include/      # Header files
│   └── opencascade/
└── cmake/        # CMake config files
```

---

## Integrating into Your CMake Project

### Option 1: Using find_package (Recommended)

In your `~/Codes/ezdesign/CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.12)
project(ezdesign)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCASCADE
set(OpenCASCADE_DIR "~/Codes/OCCT/install-minimal/lib/cmake/opencascade")
find_package(OpenCASCADE REQUIRED)

# Your source files
set(SOURCES
    src/main.cpp
    src/geometry.cpp
    # ... other sources
)

# Create your executable or library
add_executable(ezdesign ${SOURCES})

# Link against OCCT STEP modules
target_link_libraries(ezdesign
    # Core STEP modules
    OpenCASCADE::TKDESTEP
    OpenCASCADE::TKSTEPBase
    OpenCASCADE::TKSTEPAttr
    OpenCASCADE::TKSTEP209
    OpenCASCADE::TKSTEP
    
    # Required dependencies (automatically resolved, but explicit is clearer)
    OpenCASCADE::TKXSBase
    OpenCASCADE::TKDE
    OpenCASCADE::TKTopAlgo
    OpenCASCADE::TKGeomAlgo
    OpenCASCADE::TKGeomBase
    OpenCASCADE::TKBRep
    OpenCASCADE::TKG3d
    OpenCASCADE::TKG2d
    OpenCASCADE::TKMath
    OpenCASCADE::TKernel
)

# Include directories are automatically set by OpenCASCADE targets
```

### Option 2: Manual Configuration

If `find_package` doesn't work, configure manually:

```cmake
# Set OCCT paths
set(OCCT_ROOT "~/Codes/OCCT/install-minimal")
set(OCCT_INCLUDE_DIR "${OCCT_ROOT}/include")
set(OCCT_LIB_DIR "${OCCT_ROOT}/lib")

# Include directories
include_directories(${OCCT_INCLUDE_DIR})

# Link directories
link_directories(${OCCT_LIB_DIR})

# Your executable
add_executable(ezdesign ${SOURCES})

# Link libraries (order matters - dependencies first)
target_link_libraries(ezdesign
    TKDESTEP
    TKSTEPBase
    TKSTEPAttr
    TKSTEP209
    TKSTEP
    TKXSBase
    TKDE
    TKTopAlgo
    TKGeomAlgo
    TKGeomBase
    TKBRep
    TKG3d
    TKG2d
    TKMath
    TKernel
)

# Set RPATH for runtime library loading (macOS/Linux)
if(APPLE)
    set_target_properties(ezdesign PROPERTIES
        INSTALL_RPATH "${OCCT_LIB_DIR}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(UNIX)
    set_target_properties(ezdesign PROPERTIES
        INSTALL_RPATH "${OCCT_LIB_DIR}"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()
```

---

## Using STEP Export in Your Code

### Basic Example: Export a Shape to STEP

```cpp
#include <STEPControl_Writer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopoDS_Shape.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <iostream>

bool exportToStep(const TopoDS_Shape& shape, const std::string& filename)
{
    try {
        // Create STEP writer
        STEPControl_Writer writer;
        
        // Transfer shape to STEP format
        IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
        
        if (status != IFSelect_RetDone) {
            std::cerr << "Error: Failed to transfer shape to STEP" << std::endl;
            return false;
        }
        
        // Write STEP file
        status = writer.Write(filename.c_str());
        
        if (status != IFSelect_RetDone) {
            std::cerr << "Error: Failed to write STEP file" << std::endl;
            return false;
        }
        
        std::cout << "Successfully exported to: " << filename << std::endl;
        return true;
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT exception: " << e.GetMessageString() << std::endl;
        return false;
    }
}

// Example usage
int main()
{
    // Create a simple box (replace with your geometry)
    BRepPrimAPI_MakeBox boxMaker(10.0, 20.0, 30.0);
    TopoDS_Shape box = boxMaker.Shape();
    
    // Export to STEP
    if (exportToStep(box, "output.step")) {
        std::cout << "Export successful!" << std::endl;
    }
    
    return 0;
}
```

### Advanced Example: Export Multiple Shapes

```cpp
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <vector>

bool exportMultipleShapes(const std::vector<TopoDS_Shape>& shapes, 
                         const std::string& filename)
{
    try {
        STEPControl_Writer writer;
        
        // Transfer each shape
        for (const auto& shape : shapes) {
            IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
            if (status != IFSelect_RetDone) {
                std::cerr << "Warning: Failed to transfer one shape" << std::endl;
                continue;
            }
        }
        
        // Write all shapes to one STEP file
        IFSelect_ReturnStatus status = writer.Write(filename.c_str());
        return (status == IFSelect_RetDone);
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OCCT exception: " << e.GetMessageString() << std::endl;
        return false;
    }
}
```

### Converting Your Topology to OCCT Shapes

**For detailed conversion from ezdesign's topology/geometry structure, see:**
**[`EZDESIGN_TO_OCCT_CONVERSION_PROPOSAL.md`](EZDESIGN_TO_OCCT_CONVERSION_PROPOSAL.md)**

The conversion proposal provides:
- Complete analysis of ezdesign's data structure (Body → Shell → Face → Loop → HalfEdge → Edge → Vertex)
- Detailed algorithms for converting B-spline surfaces and curves
- Step-by-step conversion from each topology element
- Code examples and implementation details
- Edge case handling and performance considerations

**Quick Overview:**

ezdesign uses a half-edge data structure with:
- **B-spline surfaces** on faces (3D control points, U/V basis)
- **B-spline curves** on half-edges (2D parametric coordinates on face surface)
- **Circular linked lists** of half-edges forming loops
- **Pointers** between topology elements (next, previous, opposite)

The conversion follows a bottom-up approach:
1. Convert vertices (3D points)
2. Convert half-edges to edges (2D curves → 3D curves via surface evaluation)
3. Convert loops to wires (traverse half-edge chain)
4. Convert faces (surface + wires)
5. Convert shells (collections of faces)
6. Convert bodies (shells → solids or compounds)

---

## Complete Project Structure Example

```
~/Codes/ezdesign/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── geometry.cpp
│   ├── geometry.h
│   └── step_export.cpp      # STEP export functionality
├── include/
│   └── step_export.h
└── build/
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.12)
project(ezdesign)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCASCADE
set(OpenCASCADE_DIR "$ENV{HOME}/Codes/OCCT/install-minimal/lib/cmake/opencascade")
find_package(OpenCASCADE REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/geometry.cpp
    src/step_export.cpp
)

# Headers
set(HEADERS
    include/step_export.h
)

# Create executable
add_executable(ezdesign ${SOURCES} ${HEADERS})

# Link OCCT
target_link_libraries(ezdesign
    OpenCASCADE::TKDESTEP
    OpenCASCADE::TKSTEPBase
    OpenCASCADE::TKSTEPAttr
    OpenCASCADE::TKSTEP209
    OpenCASCADE::TKSTEP
    OpenCASCADE::TKXSBase
    OpenCASCADE::TKDE
    OpenCASCADE::TKTopAlgo
    OpenCASCADE::TKGeomAlgo
    OpenCASCADE::TKGeomBase
    OpenCASCADE::TKBRep
    OpenCASCADE::TKG3d
    OpenCASCADE::TKG2d
    OpenCASCADE::TKMath
    OpenCASCADE::TKernel
)
```

**include/step_export.h:**
```cpp
#ifndef STEP_EXPORT_H
#define STEP_EXPORT_H

#include <string>
#include <TopoDS_Shape.hxx>

class StepExporter {
public:
    static bool exportShape(const TopoDS_Shape& shape, 
                           const std::string& filename);
    static bool exportShapes(const std::vector<TopoDS_Shape>& shapes,
                            const std::string& filename);
};

#endif
```

**src/step_export.cpp:**
```cpp
#include "step_export.h"
#include <STEPControl_Writer.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <iostream>

bool StepExporter::exportShape(const TopoDS_Shape& shape, 
                               const std::string& filename)
{
    try {
        STEPControl_Writer writer;
        IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);
        
        if (status != IFSelect_RetDone) {
            return false;
        }
        
        status = writer.Write(filename.c_str());
        return (status == IFSelect_RetDone);
    }
    catch (...) {
        return false;
    }
}

bool StepExporter::exportShapes(const std::vector<TopoDS_Shape>& shapes,
                                const std::string& filename)
{
    try {
        STEPControl_Writer writer;
        
        for (const auto& shape : shapes) {
            writer.Transfer(shape, STEPControl_AsIs);
        }
        
        IFSelect_ReturnStatus status = writer.Write(filename.c_str());
        return (status == IFSelect_RetDone);
    }
    catch (...) {
        return false;
    }
}
```

---

## Building Your Project

```bash
cd ~/Codes/ezdesign
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./ezdesign
```

---

## Troubleshooting

### Issue: Cannot find OpenCASCADE

**Solution:** Set the `OpenCASCADE_DIR` CMake variable:
```bash
cmake -DOpenCASCADE_DIR=~/Codes/OCCT/install-minimal/lib/cmake/opencascade ..
```

### Issue: Missing symbols at link time

**Solution:** Ensure all required OCCT libraries are linked. The minimal build requires:
- TKDESTEP, TKSTEPBase, TKSTEPAttr, TKSTEP209, TKSTEP
- TKXSBase, TKDE
- TKTopAlgo, TKGeomAlgo, TKGeomBase
- TKBRep, TKG3d, TKG2d
- TKMath, TKernel

### Issue: Runtime library loading fails (macOS/Linux)

**Solution:** Set RPATH or set library path:
```bash
# macOS
export DYLD_LIBRARY_PATH=~/Codes/OCCT/install-minimal/lib:$DYLD_LIBRARY_PATH

# Linux
export LD_LIBRARY_PATH=~/Codes/OCCT/install-minimal/lib:$LD_LIBRARY_PATH
```

Or use CMake RPATH settings (see Option 2 above).

### Issue: Headers not found

**Solution:** Ensure include path is set:
```cmake
include_directories(${OpenCASCADE_INCLUDE_DIR})
# or
target_include_directories(your_target PRIVATE ${OpenCASCADE_INCLUDE_DIR})
```

### Issue: STEP file is empty or invalid

**Solution:** 
1. Check that the shape is valid: `!shape.IsNull()`
2. Verify transfer status: `writer.Transfer()` returns `IFSelect_RetDone`
3. Check write status: `writer.Write()` returns `IFSelect_RetDone`

---

## Additional Resources

- OCCT Documentation: See `dox/` directory in OCCT source
- STEP Control API: `src/STEPControl/STEPControl_Writer.hxx`
- Examples: `samples/` directory in OCCT source
- Test program: `adm/cmake/test_minimal_step_export.cxx`

---

## Quick Reference: Key Classes

- **`STEPControl_Writer`**: Main class for writing STEP files
- **`TopoDS_Shape`**: OCCT's shape representation (base class)
- **`TopoDS_Solid`**, **`TopoDS_Face`**, **`TopoDS_Edge`**: Specific shape types
- **`BRepBuilderAPI_*`**: Classes for building shapes programmatically
- **`IFSelect_ReturnStatus`**: Return status enumeration

---

## Profile Comparison

| Profile | Size | CAF Support | Use Case |
|---------|------|------------|----------|
| `step-export-minimal` | ~80-120MB | No | Basic shape export only |
| `step-export` | ~150-200MB | Yes | Full STEP with colors, layers, names |

For most use cases, `step-export-minimal` is sufficient if you only need geometry export.

