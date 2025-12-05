# Project Context

## Purpose
Open CASCADE Technology (OCCT) is a software development platform providing services for 3D surface and solid modeling, CAD data exchange, and visualization. The project is designed for developing software dealing with 3D modeling (CAD), manufacturing/measuring (CAM), or numerical simulation (CAE).

Primary goals:
- Provide comprehensive C++ libraries for geometric modeling and CAD operations
- Support multiple CAD data exchange formats (STEP, IGES, etc.)
- Enable 3D visualization and rendering capabilities
- Maintain cross-platform compatibility (Windows, Linux, macOS, Android, iOS, WebAssembly)

## Tech Stack
- **Language**: C++ (C++17, C++20, C++23, or C++26 - configurable via CMake)
- **Build System**: CMake (minimum version 3.10)
- **Package Management**: Optional vcpkg support for third-party dependencies
- **Third-Party Libraries**:
  - Tcl/Tk (mandatory for Draw Harness)
  - FreeType (optional, for text rendering)
  - OpenGL/OpenGL ES 2.0 (for visualization)
  - VTK (optional, for VTK integration)
  - Various optional libraries: FreeImage, FFmpeg, OpenVR, RapidJSON, Draco, TBB, Eigen, Jemalloc
- **Documentation**: Doxygen (for generating API documentation)
- **Testing**: Google Test (GTest) - optional, enabled via `BUILD_GTEST`

## Project Conventions

### Code Style
- **File Extensions**:
  - Header files: `.hxx`
  - Source files: `.cxx`
  - CMake files: `.cmake`
- **Header Guards**: Use `_` prefix and `_HeaderFile` suffix (e.g., `_Standard_Type_HeaderFile`)
- **Copyright Headers**: All source files include copyright notice with LGPL 2.1 license information
- **Naming Conventions**:
  - Classes use PascalCase (e.g., `Standard_Type`, `Handle_Standard_Transient`)
  - Functions use PascalCase (e.g., `Standard_Integer`, `DynamicType()`)
  - OCCT-specific types: `Standard_Integer`, `Standard_Real`, `Standard_Boolean`, `Standard_CString`
- **Memory Management**: 
  - Handle-based smart pointers using `Handle()` template
  - Classes inherit from `Standard_Transient` for reference counting
  - RTTI macros: `DEFINE_STANDARD_RTTIEXT`, `DEFINE_STANDARD_RTTI_INLINE`
- **Code Organization**:
  - Modular structure with toolkits organized into packages
  - Each toolkit has `FILES.cmake`, `PACKAGES.cmake`, and `EXTERNLIB.cmake` files
  - Headers collected into unified include directory during build

### Architecture Patterns
- **Modular Design**: 
  - Seven main modules: FoundationClasses, ModelingData, ModelingAlgorithms, Visualization, ApplicationFramework, DataExchange, Draw
  - Each module contains multiple toolkits (packages)
  - Toolkits can be selectively built via CMake options
- **Toolkit Structure**:
  - Each toolkit is a collection of related packages
  - Packages contain related classes and functionality
  - Dependencies managed through `EXTERNLIB.cmake` and `PACKAGES.cmake`
- **Build Configuration**:
  - Shared or static library builds (configurable)
  - Multiple build configurations: Release, Debug, RelWithDebInfo
  - Platform-specific output directories (Windows: `win64/clang/lib`, Unix: `lib`)
  - Optional precompiled headers support
- **Installation Layouts**:
  - Windows layout: traditional structure with platform/compiler subdirectories
  - Unix layout: standard FHS structure (`bin/`, `lib/`, `include/`, `share/`)
  - Vcpkg layout: vcpkg-compatible structure

### Testing Strategy
- **Framework**: Google Test (GTest) - optional, enabled via `BUILD_GTEST` CMake option
- **Test Organization**: 
  - Tests organized in `tests/` directory by category (boolean, blend, bugs, etc.)
  - Each toolkit can have `GTests/` subdirectory with test files
  - Test files use `.cxx` extension
- **Test Execution**: Tests run via CMake's `ctest` or directly through GTest executables
- **Test Data**: Test cases stored in `tests/` with various formats (BREP, STEP, IGES, etc.)

### Git Workflow
- **Branching**: Standard Git workflow (master/main branch for releases)
- **Version Management**: Version defined in `adm/cmake/version.cmake` (major.minor.maintenance format)
- **Release Process**: 
  - Certified versions available as Git snapshots, source archives, or binary packages
  - Releases published on GitHub and Open CASCADE development portal
- **Issue Tracking**: 
  - GitHub Issues: https://github.com/Open-Cascade-SAS/OCCT/issues
  - OCCT Tracker: https://tracker.dev.opencascade.org/
- **Documentation**: Markdown documentation in `dox/` directory, generated HTML/PDF via Doxygen

## Domain Context
- **CAD/CAM/CAE Domain**: The project operates in the Computer-Aided Design, Manufacturing, and Engineering domain
- **Geometric Modeling**: Core functionality includes:
  - B-Rep (Boundary Representation) modeling
  - Surface and solid geometry operations
  - Boolean operations (union, intersection, difference)
  - Feature modeling (fillets, chamfers, etc.)
  - Mesh generation and manipulation
- **Data Exchange**: Support for industry-standard formats:
  - STEP (ISO 10303)
  - IGES
  - STL
  - VRML
  - BREP (OCCT native format)
- **Visualization**: 3D rendering capabilities with:
  - OpenGL/OpenGL ES rendering backends
  - Interactive selection and highlighting
  - Hidden line removal (HLR)
  - Shader-based rendering
- **OCAF (Open CASCADE Application Framework)**: Document-based architecture for CAD applications
- **Draw Harness**: Tcl-based command-line interface for testing and demonstration

## Important Constraints
- **License**: LGPL 2.1 with special exception (see `OCCT_LGPL_EXCEPTION.txt`)
  - Static linking has limitations under LGPL
  - Commercial licensing available for proprietary static linking
- **Platform Support**: Must maintain compatibility across:
  - Windows (MSVC, MinGW)
  - Linux (various distributions)
  - macOS (Clang)
  - Android
  - iOS
  - WebAssembly (Emscripten)
- **Binary Compatibility**: OCCT must be rebuilt on target platform for binary compatibility
- **C++ Standard**: Minimum C++17, with options for C++20, C++23, or C++26
- **Memory Management**: Handle-based reference counting - avoid raw pointers for managed objects
- **Exception Safety**: Code should handle exceptions properly, with optional exception disabling in Release builds
- **Unicode Support**: Windows builds require UNICODE/_UNICODE defines

## External Dependencies
- **Mandatory**:
  - Tcl (for Draw Harness command-line interface)
  - CMake 3.10+ (build system)
- **Optional but Commonly Used**:
  - Tk (for Draw Harness GUI)
  - FreeType (text rendering)
  - OpenGL or OpenGL ES 2.0 (visualization)
- **Platform-Specific**:
  - Xlib (Linux/Unix for windowing)
  - Cocoa (macOS for windowing)
  - Win32 API (Windows)
- **Optional Features**:
  - VTK (for VTK integration toolkit)
  - FreeImage (image I/O)
  - FFmpeg (video export)
  - OpenVR (VR support)
  - RapidJSON (JSON parsing, for glTF)
  - Draco (geometry compression)
  - TBB (Intel Threading Building Blocks for parallel processing)
  - Eigen (linear algebra)
  - Jemalloc (alternative memory allocator)
- **Build Tools**:
  - Doxygen 1.8.4+ (documentation generation)
  - Bison/Flex (for parser generation in TKMath and StepFile toolkits)
  - Python (for build scripts)
- **Package Managers**:
  - vcpkg (optional, for managing third-party dependencies)
