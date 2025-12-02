# Design: JSON to STEP Converter

## Context

This feature adds the ability to read topology/geometry data from JSON format (ezdesign format) and convert it to STEP files using OCCT. The JSON format uses a half-edge data structure with B-spline surfaces and curves.

## Goals

1. **Read JSON format**: Parse ezdesign JSON structure (Body → Shell → Face → Loop → HalfEdge → Edge → Vertex)
2. **Convert to OCCT**: Transform JSON topology/geometry to OCCT `TopoDS_Shape` format
3. **Export to STEP**: Write converted shapes to STEP files
4. **Provide API**: Both command-line tool and C++ API for programmatic use

## Non-Goals

- Reading STEP files (already supported by OCCT)
- Converting from other formats to JSON
- Supporting other JSON topology formats (only ezdesign format)
- Visualization of converted shapes (use existing OCCT visualization)

## Architecture Decisions

### 1. JSON Library Choice

**Decision**: Use nlohmann/json as optional dependency.

**Rationale**: 
- nlohmann/json has more intuitive API and better error messages
- Available via package managers (Homebrew on macOS, apt/yum on Linux, vcpkg on Windows)
- Header-only library, easy to integrate
- Better developer experience for JSON parsing

**Dependency Detection Strategy**:
- Use CMake `find_package(nlohmann_json)` to detect system-installed version
- Support Homebrew installation on macOS (`/opt/homebrew/include` or `/usr/local/include`)
- Support vcpkg on Windows
- Support system package managers on Linux

### 2. Code Organization

**Decision**: Place in `tools/json2step/` as standalone tool.

**Rationale**:
- Simpler structure (standalone tool)
- Faster to implement (no toolkit scaffolding)
- Clear separation: tool vs library
- Can be refactored to toolkit later if library API is needed

**Alternative Considered**: Create `TKJsonReader` toolkit (more complex, but provides reusable library)

### 3. Conversion Pipeline

**Decision**: Two-stage conversion: JSON → C++ structures → OCCT shapes

**Rationale**:
- Separation of concerns: parsing vs conversion
- Easier to test and debug
- Allows future optimization (caching, lazy evaluation)

**Pipeline**:
```
JSON File
  ↓
JSON Parser (nlohmann/json)
  ↓
C++ Data Structures (EzBody, EzShell, etc.)
  ↓
OCCT Converter (EzDesignToOCCTConverter)
  ↓
TopoDS_Shape
  ↓
STEPControl_Writer
  ↓
STEP File
```

### 4. Error Handling Strategy

**Decision**: Fail fast with clear, detailed error messages. Use exceptions for fatal errors, return codes for recoverable errors.

**Rationale**:
- Preserves data integrity (no silent corruption)
- Easier to debug (errors are explicit)
- Simpler implementation
- User knows exactly what's wrong
- OCCT uses exceptions (`Standard_Failure`)
- Command-line tool should return exit codes
- Provide detailed error messages with context

**Error Reporting Approach**:
- Add validation phase before conversion
- Report all errors at once (not just first error)
- Provide suggestions for fixes when possible
- Example: "Missing opposite half-edge for edge 42. Expected half-edge with opposite=42."

### 5. Geometry Conversion Approach

**Decision**: Bottom-up conversion (Vertex → Edge → Wire → Face → Shell → Body)

**Rationale**:
- Dependencies flow upward (edges need vertices, wires need edges, etc.)
- Easier to validate at each level
- Matches OCCT's topology hierarchy

### 6. B-spline Conversion

**Decision**: Direct conversion from JSON control points to OCCT B-spline objects.

**Rationale**:
- JSON format stores complete B-spline data (control points, knots, degrees)
- No approximation needed
- Preserves exact geometry

**Key Algorithms**:
- Reshape flattened control point arrays
- Compute knot multiplicities from knot vectors
- Handle rational vs non-rational cases

### 7. Half-Edge Traversal and Orientation

**Decision**: Use `next` pointer to traverse circular linked list, respecting ezdesign's orientation convention.

**Rationale**:
- JSON structure provides `next` pointer for each half-edge
- Simple iteration until back to start
- Handles both closed and open loops

**ezdesign Orientation Convention**:
- Half-edges are arranged in **CCW (counter-clockwise) order** with respect to the **face**'s outward normal direction
- This follows the **right-hand rule**: thumb points in face outward normal direction, fingers curl in half-edge direction (CCW)
- Face has `is_surface_normal_same` flag indicating if surface normal matches face outward normal:
  - **If `true` (same)**: Half-edges in UV (2D parametric) domain form **CCW loops**
  - **If `false` (opposite)**: Half-edges in UV domain form **CW loops**, but this still represents **CCW for the face** (because surface normal is opposite, right-hand rule still applies)
- This convention ensures consistent face orientation regardless of surface normal direction
- Half-edges and curves are always oriented in the same direction, regardless of the surface normal direction

### 8. 2D to 3D Curve Mapping

**Decision**: Sample 2D parametric curve and evaluate on surface to get 3D curve (sampling-based conversion).

**Rationale**:
- Simpler implementation
- Works for any 2D curve type
- Fast to implement
- Good enough for most use cases
- HalfEdge curves are 2D parametric (u, v on face surface)
- Need 3D curve for edge geometry
- Surface evaluation: `surface->Value(u, v)` → 3D point
- Create interpolated 3D B-spline from sampled points

**Implementation Details**:
- Use adaptive sampling (more samples for complex curves)
- Default: 20-50 samples based on curve degree/length
- Document that it's an approximation
- Can be improved later if precision issues arise

**Alternative Considered**: Exact mathematical mapping (more complex, preserves exact geometry but requires significant research/development time)

## Data Structures

### JSON Topology Elements

```cpp
struct EzVertex {
    int id;
    double position[3];
    int half_edge_id;
};

struct EzHalfEdge {
    int id;
    int edge_id;
    int vertex_id;  // Start vertex
    int loop_id;
    int next_id;
    int previous_id;
    int opposite_id;
    CurveData curve_data;  // 2D B-spline (optional)
};

struct EzEdge {
    int id;
    int half_edge_id;  // One of the two half-edges
};

struct EzLoop {
    int id;
    int face_id;
    int half_edge_id;  // Starting half-edge
};

struct EzFace {
    int id;
    int shell_id;
    std::vector<int> loop_ids;
    SurfaceData surface_data;  // 3D B-spline surface
    bool is_surface_normal_same;  // If true: surface normal == face outward normal
                                   // If false: surface normal opposite to face outward normal
                                   // Affects UV domain orientation: true=CCW in UV, false=CW in UV (but CCW for face)
    // Note: is_normal_outward field in JSON is ignored - not used in conversion
};

struct EzShell {
    int id;
    int body_id;
    std::vector<int> face_ids;
};

struct EzBody {
    int id;
    std::vector<int> shell_ids;
};
```

### Geometry Data Structures

```cpp
struct ControlPoints {
    std::vector<double> data;  // Flattened array
    int dimension;  // 2 or 3
    int number_u_points;  // For surfaces: U direction
    int number_v_points;  // For surfaces: V direction, for curves: always 2
    bool is_rational;
};

struct Basis {
    int degree;
    std::vector<double> knot_vector;
    struct {
        double minimum;
        double maximum;
    } bounds;
};

struct CurveData {
    ControlPoints control_points;
    Basis basis;
};

struct SurfaceData {
    ControlPoints control_points;
    Basis u_basis;
    Basis v_basis;
    // Note: is_normal_outward and trimming_loop fields in JSON are ignored - not used in conversion
    // Face boundaries come from Loop topology elements, not trimming_loop
};
```

## Class Design

**Code Location**: `tools/json2step/`

### EzDesignJsonReader

**Purpose**: Parse JSON file and populate C++ data structures.

**Location**: `tools/json2step/EzDesignJsonReader.hxx` / `.cxx`

```cpp
class EzDesignJsonReader {
public:
    bool ReadFile(const std::string& filename);
    const EzBody& GetBody() const;
    // ... accessors for topology elements
    
    // Validation and error reporting
    bool Validate() const;
    std::vector<std::string> GetErrors() const;
    
private:
    EzBody myBody;
    std::map<int, EzVertex> myVertices;
    std::map<int, EzEdge> myEdges;
    // ... other topology maps
    std::vector<std::string> myErrors;  // Collected validation errors
};
```

### EzDesignToOCCTConverter

**Purpose**: Convert C++ data structures to OCCT shapes.

**Location**: `tools/json2step/EzDesignToOCCTConverter.hxx` / `.cxx`

```cpp
class EzDesignToOCCTConverter {
public:
    TopoDS_Shape ConvertBody(const EzBody& body);
    
    // Error reporting
    bool HasErrors() const;
    std::vector<std::string> GetErrors() const;
    
private:
    // Geometry conversion
    Handle(Geom_BSplineSurface) convertSurface(const SurfaceData& data);
    Handle(Geom2d_BSplineCurve) convertCurve2D(const CurveData& data);
    Handle(Geom_BSplineCurve) convertCurve3D(
        const Handle(Geom2d_BSplineCurve)& curve2d,
        const Handle(Geom_BSplineSurface)& surface);
    
    // Topology conversion
    TopoDS_Vertex convertVertex(const EzVertex& vertex);
    TopoDS_Edge convertHalfEdge(const EzHalfEdge& halfEdge, 
                                const Handle(Geom_BSplineSurface)& surface,
                                bool isSurfaceNormalSame);
    TopoDS_Wire convertLoop(const EzLoop& loop, 
                          const Handle(Geom_BSplineSurface)& surface,
                          bool isSurfaceNormalSame);
    TopoDS_Face convertFace(const EzFace& face);
    TopoDS_Shell convertShell(const EzShell& shell);
    TopoDS_Shape convertBody(const EzBody& body);
    
    // Helper methods
    void computeKnotMultiplicities(...);
    TColgp_Array2OfPnt reshapeControlPoints(...);
    int computeSampleCount(const Handle(Geom2d_BSplineCurve)& curve2d);  // Adaptive sampling
    
    std::vector<std::string> myErrors;  // Collected conversion errors
};
```

### json2step Command-Line Tool

**Purpose**: Main executable for JSON to STEP conversion.

**Location**: `tools/json2step/json2step.cxx`

```cpp
int main(int argc, char* argv[])
{
    // Parse command-line arguments
    // Read JSON file
    // Validate data
    // Convert to OCCT shapes
    // Export to STEP
    // Report errors and exit codes
}
```

## Edge Cases and Error Handling

### 1. Malformed JSON
- **Detection**: JSON parser will throw exceptions
- **Handling**: Catch and provide user-friendly error message with line number

### 2. Missing Required Fields
- **Detection**: Check for required fields after parsing
- **Handling**: Return error with specific field name

### 3. Invalid Topology
- **Detection**: Validate topology during conversion (e.g., loop not closed)
- **Handling**: Report error, skip invalid elements, or fail fast

### 4. Degenerate Geometry
- **Detection**: Check for degenerate edges (same start/end vertex)
- **Handling**: Create degenerate edge or skip

### 5. Open Loops
- **Detection**: Check if loop closes (half-edge chain returns to start)
- **Handling**: Report error (open loops invalid in B-Rep)

### 6. Invalid B-spline Data
- **Detection**: Validate control points, knots, degrees
- **Handling**: Report error with specific issue

## Performance Considerations

### Optimization Strategies

1. **Caching**: Cache converted OCCT objects to avoid redundant conversions
2. **Lazy Evaluation**: Only convert geometry when needed
3. **Batch Processing**: Convert all vertices first, then edges, etc.
4. **Memory Management**: Use OCCT handles (smart pointers)

### Expected Performance

- Small models (< 100 faces): < 1 second
- Medium models (100-1000 faces): 1-10 seconds
- Large models (> 1000 faces): 10+ seconds

Bottlenecks:
- JSON parsing: Fast (nlohmann/json is efficient)
- B-spline conversion: Moderate (reshaping arrays)
- Surface evaluation for 3D curves: Potentially slow for complex curves

## Testing Strategy

### Unit Tests

1. **JSON Parsing**: Test each topology element parser
2. **Geometry Conversion**: Test B-spline surface/curve conversion
3. **Topology Conversion**: Test each conversion level (vertex → edge → wire → face)

### Integration Tests

1. **End-to-End**: JSON → STEP → Read back with OCCT
2. **Round-Trip**: Convert, export, import, compare

### Test Data

- Simple box (single body, single shell, 6 faces)
- Complex model with multiple shells
- Model with holes (inner loops)
- Model with B-spline surfaces

## Dependencies

### Required OCCT Modules

- `TKernel` - Core types
- `TKMath` - Mathematics
- `TKG2d`, `TKG3d` - 2D/3D geometry
- `TKGeomBase` - Geometry base classes
- `TKGeomAlgo` - Geometry algorithms
- `TKBRep` - Boundary representation
- `TKTopAlgo` - Topology algorithms
- `TKDESTEP` - STEP export

### External Dependencies

- **nlohmann/json** (optional):
  - Header-only JSON library
  - Available via Homebrew on macOS: `brew install nlohmann-json`
  - Available via vcpkg: `vcpkg install nlohmann-json`
  - Available via system package managers on Linux
  - Detected via CMake `find_package(nlohmann_json)`
- Standard C++ library

## Migration and Compatibility

### Backward Compatibility

- No impact on existing code (additive feature)
- Optional dependency (can be disabled in build)

### Future Extensions

- Support for other JSON formats
- Support for reading STEP and converting to JSON
- Support for other export formats (IGES, BREP)

## Resolved Decisions

1. ~~**JSON Library**: Final decision on RapidJSON vs nlohmann/json?~~ **RESOLVED**: Use nlohmann/json

2. ~~**Code Location**: Toolkit vs utility directory?~~ **RESOLVED**: Place in `tools/json2step/` as standalone tool
   - Simpler structure and faster to implement
   - Can be refactored to toolkit later if library API is needed

3. ~~**Error Recovery**: Should converter attempt to fix common errors?~~ **RESOLVED**: Fail fast with clear, detailed error messages
   - Preserves data integrity
   - Report all errors at once with suggestions for fixes
   - Can add optional "repair mode" later if needed

4. ~~**Performance**: Is sampling-based 3D curve conversion acceptable?~~ **RESOLVED**: Use sampling-based conversion
   - Simpler implementation, good enough for most use cases
   - Use adaptive sampling (20-50 samples based on curve complexity)
   - Can be improved later if precision issues arise

