# Conversion Proposal: ezdesign Topology/Geometry to OCCT TopoDS_Shape

## Executive Summary

This document proposes a detailed conversion strategy from ezdesign's topology/geometry data structure to OCCT's `TopoDS_Shape` format for STEP file export. The conversion handles B-spline surfaces and curves, half-edge topology, and maintains geometric accuracy.

## ezdesign Data Structure Analysis

Based on the JSON structure analysis, ezdesign uses the following topology hierarchy:

```
Body
  └── Shell(s)
        └── Face(s)
              └── Loop(s)
                    └── HalfEdge(s) [circular linked list]
                          ├── Edge (shared between two HalfEdges)
                          ├── Vertex (start point)
                          └── curve_data (2D B-spline curve on face surface)
              └── surface_data (3D B-spline surface)
```

### Topology Elements

1. **Body** (`type: "Body"`)
   - Contains: `shells: [shell_id, ...]`
   - Represents: A complete solid or surface model

2. **Shell** (`type: "Shell"`)
   - Contains: `faces: [face_id, ...]`, `body: body_id`
   - Represents: A connected set of faces forming a closed or open surface

3. **Face** (`type: "Face"`)
   - Contains: `loops: [loop_id, ...]`, `shell: shell_id`
   - Geometry: `surface_data` (B-spline surface)
   - Properties: `is_surface_normal_same`, `is_normal_outward`

4. **Loop** (`type: "Loop"`)
   - Contains: `half_edge: halfedge_id`, `face: face_id`
   - Represents: A closed boundary of a face (outer or inner)

5. **HalfEdge** (`type: "HalfEdge"`)
   - Contains: `edge: edge_id`, `vertex: vertex_id`, `loop: loop_id`
   - Navigation: `next: halfedge_id`, `previous: halfedge_id`, `opposite: halfedge_id`
   - Geometry: `curve_data` (2D B-spline curve, parametric on face surface)
   - Properties: `curve_data.basis.bounds.minimum/maximum` (parameter range)

6. **Edge** (`type: "Edge"`)
   - Contains: `half_edge: halfedge_id` (one of the two half-edges)
   - Represents: An undirected edge shared between two faces

7. **Vertex** (`type: "Vertex"`)
   - Contains: `position: [x, y, z]`, `half_edge: halfedge_id`
   - Represents: A 3D point

### Geometry Elements

#### B-spline Surface (Face)
```json
"surface_data": {
  "control_points": {
    "data": [x1, y1, z1, x2, y2, z2, ...],  // Flattened 3D array
    "dimension": 3,
    "number_u_points": 4,  // U direction control points
    "number_v_points": 4,  // V direction control points
    "is_rational": false
  },
  "u_basis": {
    "degree": 3,
    "knot_vector": [0, 1, 2, 3, 4, 5, 6, 7],
    "bounds": {"minimum": 3, "maximum": 4}
  },
  "v_basis": {
    "degree": 3,
    "knot_vector": [0, 1, 2, 3, 4, 5, 6, 7],
    "bounds": {"minimum": 3, "maximum": 4}
  },
  "is_normal_outward": true
}
```

#### B-spline Curve (HalfEdge - 2D parametric)
```json
"curve_data": {
  "control_points": {
    "data": [u1, v1, u2, v2, ...],  // 2D parametric coordinates
    "dimension": 2,
    "number_u_points": 1,  // Actually number of control points
    "number_v_points": 2,  // Actually second dimension (always 2 for 2D)
    "is_rational": false
  },
  "basis": {
    "degree": 1,
    "knot_vector": [0, 0, 1, 1],
    "bounds": {"minimum": 0, "maximum": 1}
  }
}
```

**Note**: The 2D curve data represents parametric coordinates (u, v) on the face's surface, not 3D coordinates.

---

## Conversion Strategy

### Phase 1: Geometry Conversion

#### 1.1 Convert B-spline Surface to `Geom_BSplineSurface`

**Input**: Face's `surface_data`
**Output**: `Handle(Geom_BSplineSurface)`

**Algorithm**:
1. Extract control points from flattened array
2. Reshape to 2D array: `TColgp_Array2OfPnt(1, numU, 1, numV)`
3. Extract knot vectors and compute multiplicities
4. Create `Geom_BSplineSurface`

**Key Considerations**:
- Control points are stored as flattened array: `[x1,y1,z1, x2,y2,z2, ...]`
- Need to reshape: `poles[i][j] = data[(i * numV + j) * 3 + {0,1,2}]`
- Knot multiplicities: For non-periodic surfaces, first and last multiplicities = degree + 1
- Parameter bounds: `u_basis.bounds` and `v_basis.bounds` define surface domain

#### 1.2 Convert B-spline Curve to `Geom2d_BSplineCurve`

**Input**: HalfEdge's `curve_data`
**Output**: `Handle(Geom2d_BSplineCurve)`

**Algorithm**:
1. Extract 2D control points: `[u1, v1, u2, v2, ...]` → `gp_Pnt2d(u, v)`
2. Extract knot vector and compute multiplicities
3. Create `Geom2d_BSplineCurve` (2D curve for face parameterization)

**Key Considerations**:
- Curve is 2D parametric (u, v coordinates on surface)
- Parameter range: `basis.bounds.minimum` to `basis.bounds.maximum`
- This curve will be used to create the 3D edge by evaluating on the surface

---

### Phase 2: Topology Conversion (Bottom-Up Approach)

#### 2.1 Convert Vertex to `TopoDS_Vertex`

**Input**: Vertex JSON object
**Output**: `TopoDS_Vertex`

```cpp
TopoDS_Vertex convertVertex(const JsonVertex& vertex) {
    gp_Pnt point(vertex.position[0], 
                 vertex.position[1], 
                 vertex.position[2]);
    BRep_Builder builder;
    TopoDS_Vertex v;
    builder.MakeVertex(v, point);
    return v;
}
```

#### 2.2 Convert HalfEdge to `TopoDS_Edge`

**Input**: HalfEdge JSON object, Face's surface
**Output**: `TopoDS_Edge`

**Algorithm**:
1. Get 2D B-spline curve from `curve_data`
2. Create `Geom2d_BSplineCurve` from 2D control points
3. Get start and end vertices
4. Create 3D curve by evaluating 2D curve on surface:
   - For each point on 2D curve: `surface->Value(u, v)` → 3D point
   - Build 3D B-spline curve from 3D points
5. Create edge from 3D curve and vertices
6. Attach 2D curve to edge for face parameterization

**Key Considerations**:
- HalfEdge has parameter range: `basis.bounds.minimum` to `basis.bounds.maximum`
- Need to map 2D parametric curve to 3D space using surface evaluation
- Edge orientation: determined by HalfEdge direction (forward vs opposite)

#### 2.3 Convert Loop to `TopoDS_Wire`

**Input**: Loop JSON object, Face's surface, HalfEdge map
**Output**: `TopoDS_Wire`

**Algorithm**:
1. Start from `loop.half_edge`
2. Traverse HalfEdge chain using `next` pointer until back to start
3. For each HalfEdge:
   - Convert to `TopoDS_Edge` (see 2.2)
   - Add to wire builder
4. Close wire if loop is closed
5. Set wire orientation (outer vs inner loop)

**Key Considerations**:
- Loop traversal: `halfedge → halfedge.next → ... → back to start`
- Wire orientation: Outer loop = forward, Inner loop = reversed
- Multiple loops per face: First loop is outer, rest are inner (holes)

#### 2.4 Convert Face to `TopoDS_Face`

**Input**: Face JSON object, all converted loops
**Output**: `TopoDS_Face`

**Algorithm**:
1. Convert surface to `Geom_BSplineSurface` (see 1.1)
2. Create face from surface: `BRepBuilderAPI_MakeFace(surface)`
3. For each loop:
   - Convert to `TopoDS_Wire` (see 2.3)
   - Add wire to face:
     - First loop: outer boundary
     - Subsequent loops: inner boundaries (holes)
4. Set face orientation based on `is_normal_outward`

**Key Considerations**:
- Face orientation: `is_normal_outward` determines if face normal matches surface normal
- Multiple loops: Outer loop defines boundary, inner loops define holes
- Surface domain: Use `u_basis.bounds` and `v_basis.bounds` for trimming

#### 2.5 Convert Shell to `TopoDS_Shell`

**Input**: Shell JSON object, all converted faces
**Output**: `TopoDS_Shell`

**Algorithm**:
1. Create empty shell: `BRep_Builder::MakeShell()`
2. For each face in shell:
   - Convert to `TopoDS_Face` (see 2.4)
   - Add face to shell
3. Check if shell is closed: `BRep_Tool::IsClosed(shell)`

**Key Considerations**:
- Shell can be open or closed
- Face orientation must be consistent (all normals pointing outward for closed shell)

#### 2.6 Convert Body to `TopoDS_Solid` or `TopoDS_Compound`

**Input**: Body JSON object, all converted shells
**Output**: `TopoDS_Solid` or `TopoDS_Compound`

**Algorithm**:
1. If body has one closed shell:
   - Convert shell to solid: `BRepBuilderAPI_MakeSolid(shell)`
   - Return `TopoDS_Solid`
2. If body has multiple shells or open shells:
   - Create compound: `BRep_Builder::MakeCompound()`
   - Add all shells to compound
   - Return `TopoDS_Compound`

**Key Considerations**:
- Single closed shell → Solid
- Multiple shells or open shells → Compound
- Void shells (holes) need special handling

---

## Implementation Details

### Data Structures

```cpp
// ezdesign topology elements (parsed from JSON)
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
    CurveData curve_data;  // 2D B-spline
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
    bool is_surface_normal_same;
    bool is_normal_outward;
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

### Conversion Classes

```cpp
class EzDesignToOCCTConverter {
public:
    // Main conversion entry point
    TopoDS_Shape convertBody(const EzBody& body);
    
private:
    // Geometry conversion
    Handle(Geom_BSplineSurface) convertSurface(const SurfaceData& surfData);
    Handle(Geom2d_BSplineCurve) convertCurve2D(const CurveData& curveData);
    Handle(Geom_BSplineCurve) convertCurve3D(
        const Handle(Geom2d_BSplineCurve)& curve2d,
        const Handle(Geom_BSplineSurface)& surface);
    
    // Topology conversion
    TopoDS_Vertex convertVertex(const EzVertex& vertex);
    TopoDS_Edge convertHalfEdge(const EzHalfEdge& halfEdge, 
                                const Handle(Geom_BSplineSurface)& surface);
    TopoDS_Wire convertLoop(const EzLoop& loop, 
                            const Handle(Geom_BSplineSurface)& surface);
    TopoDS_Face convertFace(const EzFace& face);
    TopoDS_Shell convertShell(const EzShell& shell);
    TopoDS_Shape convertBody(const EzBody& body);
    
    // Helper methods
    void computeKnotMultiplicities(const std::vector<double>& knots,
                                   int degree,
                                   std::vector<int>& multiplicities);
    TColgp_Array2OfPnt reshapeControlPoints(
        const std::vector<double>& flatData,
        int numU, int numV);
    TColgp_Array1OfPnt2d reshapeControlPoints2D(
        const std::vector<double>& flatData,
        int numPoints);
};
```

---

## Detailed Conversion Algorithms

### Algorithm 1: Convert B-spline Surface

```cpp
Handle(Geom_BSplineSurface) EzDesignToOCCTConverter::convertSurface(
    const SurfaceData& surfData)
{
    const auto& cp = surfData.control_points;
    const auto& uBasis = surfData.u_basis;
    const auto& vBasis = surfData.v_basis;
    
    // 1. Reshape control points from flat array to 2D array
    int numU = cp.number_u_points;
    int numV = cp.number_v_points;
    TColgp_Array2OfPnt poles(1, numU, 1, numV);
    
    for (int i = 0; i < numU; i++) {
        for (int j = 0; j < numV; j++) {
            int idx = (i * numV + j) * 3;
            gp_Pnt point(cp.data[idx], 
                        cp.data[idx + 1], 
                        cp.data[idx + 2]);
            poles.SetValue(i + 1, j + 1, point);
        }
    }
    
    // 2. Process U-direction knots and multiplicities
    TColStd_Array1OfReal uKnots(1, uBasis.knot_vector.size());
    TColStd_Array1OfInteger uMults(1, uBasis.knot_vector.size());
    computeKnotMultiplicities(uBasis.knot_vector, 
                             uBasis.degree, 
                             uMults);
    for (size_t i = 0; i < uBasis.knot_vector.size(); i++) {
        uKnots.SetValue(i + 1, uBasis.knot_vector[i]);
    }
    
    // 3. Process V-direction knots and multiplicities
    TColStd_Array1OfReal vKnots(1, vBasis.knot_vector.size());
    TColStd_Array1OfInteger vMults(1, vBasis.knot_vector.size());
    computeKnotMultiplicities(vBasis.knot_vector, 
                             vBasis.degree, 
                             vMults);
    for (size_t i = 0; i < vBasis.knot_vector.size(); i++) {
        vKnots.SetValue(i + 1, vBasis.knot_vector[i]);
    }
    
    // 4. Create surface
    Handle(Geom_BSplineSurface) surface;
    if (cp.is_rational) {
        // Handle rational case (weights needed)
        // For now, assume non-rational
        surface = new Geom_BSplineSurface(poles, uKnots, vKnots, 
                                         uMults, vMults,
                                         uBasis.degree, vBasis.degree);
    } else {
        surface = new Geom_BSplineSurface(poles, uKnots, vKnots, 
                                         uMults, vMults,
                                         uBasis.degree, vBasis.degree);
    }
    
    return surface;
}
```

### Algorithm 2: Convert 2D B-spline Curve

```cpp
Handle(Geom2d_BSplineCurve) EzDesignToOCCTConverter::convertCurve2D(
    const CurveData& curveData)
{
    const auto& cp = curveData.control_points;
    const auto& basis = curveData.basis;
    
    // 1. Reshape 2D control points
    int numPoints = cp.number_u_points;  // Actually number of control points
    TColgp_Array1OfPnt2d poles(1, numPoints);
    
    for (int i = 0; i < numPoints; i++) {
        int idx = i * 2;  // 2D: u, v coordinates
        gp_Pnt2d point(cp.data[idx], cp.data[idx + 1]);
        poles.SetValue(i + 1, point);
    }
    
    // 2. Process knots and multiplicities
    TColStd_Array1OfReal knots(1, basis.knot_vector.size());
    TColStd_Array1OfInteger mults(1, basis.knot_vector.size());
    computeKnotMultiplicities(basis.knot_vector, basis.degree, mults);
    
    for (size_t i = 0; i < basis.knot_vector.size(); i++) {
        knots.SetValue(i + 1, basis.knot_vector[i]);
    }
    
    // 3. Create 2D curve
    Handle(Geom2d_BSplineCurve) curve2d = 
        new Geom2d_BSplineCurve(poles, knots, mults, basis.degree);
    
    return curve2d;
}
```

### Algorithm 3: Convert 2D Curve to 3D Edge

```cpp
TopoDS_Edge EzDesignToOCCTConverter::convertHalfEdge(
    const EzHalfEdge& halfEdge,
    const Handle(Geom_BSplineSurface)& surface)
{
    // 1. Convert 2D curve
    Handle(Geom2d_BSplineCurve) curve2d = convertCurve2D(halfEdge.curve_data);
    
    // 2. Get parameter range
    double uMin = halfEdge.curve_data.basis.bounds.minimum;
    double uMax = halfEdge.curve_data.basis.bounds.maximum;
    
    // 3. Get start and end vertices
    EzVertex startVertex = getVertex(halfEdge.vertex_id);
    EzVertex endVertex = getVertex(getNextHalfEdge(halfEdge.id).vertex_id);
    
    TopoDS_Vertex vStart = convertVertex(startVertex);
    TopoDS_Vertex vEnd = convertVertex(endVertex);
    
    // 4. Create 3D curve by sampling 2D curve on surface
    // Option A: Sample and create interpolated 3D B-spline
    int numSamples = 20;  // Adjust based on curve complexity
    TColgp_Array1OfPnt points3d(1, numSamples);
    
    for (int i = 0; i < numSamples; i++) {
        double t = uMin + (uMax - uMin) * i / (numSamples - 1);
        gp_Pnt2d p2d = curve2d->Value(t);
        gp_Pnt p3d = surface->Value(p2d.X(), p2d.Y());
        points3d.SetValue(i + 1, p3d);
    }
    
    // Create 3D B-spline curve from sampled points
    // (Simplified - in practice, use proper interpolation)
    TColStd_Array1OfReal knots3d(1, numSamples);
    TColStd_Array1OfInteger mults3d(1, numSamples);
    for (int i = 1; i <= numSamples; i++) {
        knots3d.SetValue(i, (i - 1.0) / (numSamples - 1.0));
        mults3d.SetValue(i, (i == 1 || i == numSamples) ? 2 : 1);
    }
    
    Handle(Geom_BSplineCurve) curve3d = 
        new Geom_BSplineCurve(points3d, knots3d, mults3d, 1);
    
    // 5. Create edge from 3D curve
    BRepBuilderAPI_MakeEdge edgeMaker(curve3d, vStart, vEnd);
    TopoDS_Edge edge = edgeMaker.Edge();
    
    // 6. Attach 2D curve to edge for face parameterization
    BRep_Builder builder;
    builder.UpdateEdge(edge, curve2d, surface, Precision::Confusion());
    
    return edge;
}
```

### Algorithm 4: Convert Loop to Wire

```cpp
TopoDS_Wire EzDesignToOCCTConverter::convertLoop(
    const EzLoop& loop,
    const Handle(Geom_BSplineSurface)& surface)
{
    BRepBuilderAPI_MakeWire wireMaker;
    
    // Traverse half-edge chain
    int currentHeId = loop.half_edge_id;
    int startHeId = currentHeId;
    bool isFirst = true;
    
    do {
        EzHalfEdge halfEdge = getHalfEdge(currentHeId);
        TopoDS_Edge edge = convertHalfEdge(halfEdge, surface);
        
        // Set edge orientation based on half-edge direction
        // (Forward for outer loop, may need reversal for inner loop)
        wireMaker.Add(edge);
        
        // Move to next half-edge
        currentHeId = halfEdge.next_id;
        isFirst = false;
    } while (currentHeId != startHeId);
    
    TopoDS_Wire wire = wireMaker.Wire();
    
    // Check if wire is closed
    if (!wireMaker.IsDone()) {
        // Handle error: wire not closed
        return TopoDS_Wire();
    }
    
    return wire;
}
```

### Algorithm 5: Convert Face

```cpp
TopoDS_Face EzDesignToOCCTConverter::convertFace(const EzFace& face)
{
    // 1. Convert surface
    Handle(Geom_BSplineSurface) surface = convertSurface(face.surface_data);
    
    // 2. Create face from surface
    BRepBuilderAPI_MakeFace faceMaker(surface, Precision::Confusion());
    
    // 3. Add loops (wires)
    bool isFirstLoop = true;
    for (int loopId : face.loop_ids) {
        EzLoop loop = getLoop(loopId);
        TopoDS_Wire wire = convertLoop(loop, surface);
        
        if (wire.IsNull()) continue;
        
        if (isFirstLoop) {
            // First loop is outer boundary
            faceMaker.Add(wire);
            isFirstLoop = false;
        } else {
            // Subsequent loops are inner boundaries (holes)
            faceMaker.Add(wire);
        }
    }
    
    TopoDS_Face resultFace = faceMaker.Face();
    
    // 4. Set face orientation
    if (!face.is_normal_outward) {
        resultFace.Reverse();
    }
    
    return resultFace;
}
```

### Algorithm 6: Convert Shell

```cpp
TopoDS_Shell EzDesignToOCCTConverter::convertShell(const EzShell& shell)
{
    BRep_Builder builder;
    TopoDS_Shell resultShell;
    builder.MakeShell(resultShell);
    
    // Convert and add all faces
    for (int faceId : shell.face_ids) {
        EzFace face = getFace(faceId);
        TopoDS_Face occtFace = convertFace(face);
        
        if (!occtFace.IsNull()) {
            builder.Add(resultShell, occtFace);
        }
    }
    
    // Check if shell is closed
    bool isClosed = BRep_Tool::IsClosed(resultShell);
    // Shell closure is automatically determined by OCCT
    
    return resultShell;
}
```

### Algorithm 7: Convert Body

```cpp
TopoDS_Shape EzDesignToOCCTConverter::convertBody(const EzBody& body)
{
    if (body.shell_ids.size() == 0) {
        return TopoDS_Shape();  // Empty body
    }
    
    if (body.shell_ids.size() == 1) {
        // Single shell - try to make solid
        EzShell shell = getShell(body.shell_ids[0]);
        TopoDS_Shell occtShell = convertShell(shell);
        
        if (BRep_Tool::IsClosed(occtShell)) {
            // Closed shell → Solid
            BRepBuilderAPI_MakeSolid solidMaker(occtShell);
            if (solidMaker.IsDone()) {
                return TopoDS::Solid(solidMaker.Solid());
            }
        }
        
        // If solid creation fails, return shell
        return occtShell;
    } else {
        // Multiple shells → Compound
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        
        for (int shellId : body.shell_ids) {
            EzShell shell = getShell(shellId);
            TopoDS_Shell occtShell = convertShell(shell);
            builder.Add(compound, occtShell);
        }
        
        return compound;
    }
}
```

---

## Helper Functions

### Compute Knot Multiplicities

```cpp
void EzDesignToOCCTConverter::computeKnotMultiplicities(
    const std::vector<double>& knots,
    int degree,
    TColStd_Array1OfInteger& multiplicities)
{
    // For non-periodic B-splines:
    // - First and last multiplicities = degree + 1
    // - Internal multiplicities = 1 (unless knot is repeated)
    
    int nKnots = knots.size();
    
    if (nKnots < 2) {
        // Error: insufficient knots
        return;
    }
    
    // First knot: multiplicity = degree + 1
    multiplicities.SetValue(1, degree + 1);
    
    // Last knot: multiplicity = degree + 1
    multiplicities.SetValue(nKnots, degree + 1);
    
    // Internal knots: check for repetitions
    for (int i = 2; i < nKnots; i++) {
        int mult = 1;
        // Check if knot repeats (within tolerance)
        double tol = 1e-9;
        for (int j = i + 1; j <= nKnots; j++) {
            if (std::abs(knots[i-1] - knots[j-1]) < tol) {
                mult++;
            } else {
                break;
            }
        }
        multiplicities.SetValue(i, mult);
    }
}
```

---

## Edge Cases and Special Handling

### 1. Degenerate Edges
- **Issue**: Edge with same start/end vertex
- **Solution**: Create degenerate edge using `BRepBuilderAPI_MakeEdge` with single vertex

### 2. Open Loops
- **Issue**: Loop that doesn't close (shouldn't happen in valid topology)
- **Solution**: Validate loop closure, report error if open

### 3. Face with No Loops
- **Issue**: Face without boundary definition
- **Solution**: Use natural surface boundary (surface domain)

### 4. Rational B-splines
- **Issue**: `is_rational: true` requires weights
- **Solution**: Check if weights are provided, use weighted constructor

### 5. Periodic Surfaces/Curves
- **Issue**: Surface or curve that wraps around
- **Solution**: Check knot vector pattern, set periodic flag

### 6. Surface Domain Trimming
- **Issue**: Surface bounds may not match natural domain
- **Solution**: Use `u_basis.bounds` and `v_basis.bounds` to trim surface

### 7. HalfEdge Orientation
- **Issue**: Need to determine if edge should be forward or reversed
- **Solution**: Check `is_normal_outward` and loop type (outer/inner)

---

## Performance Considerations

### Optimization Strategies

1. **Caching**: Cache converted geometry objects to avoid redundant conversions
2. **Lazy Evaluation**: Only convert geometry when needed
3. **Batch Processing**: Convert all vertices first, then edges, then faces
4. **Memory Management**: Use handles (smart pointers) for OCCT objects

### Memory Usage

- **Control Points**: Large arrays for complex surfaces (4x4 = 16 points minimum)
- **Knot Vectors**: Typically small (degree + 1 to 2*degree + 2)
- **Topology Maps**: Need to maintain ID → OCCT object mappings

---

## Testing Strategy

### Unit Tests

1. **Geometry Conversion**:
   - Test surface conversion with various degrees (1, 2, 3)
   - Test curve conversion with different parameter ranges
   - Test rational vs non-rational cases

2. **Topology Conversion**:
   - Test single face conversion
   - Test face with multiple loops (holes)
   - Test closed shell → solid conversion
   - Test open shell → shell conversion

3. **Integration Tests**:
   - Convert complete body and export to STEP
   - Verify STEP file can be read back
   - Compare geometry accuracy

### Validation

1. **Geometric Accuracy**:
   - Compare control points (should match exactly)
   - Verify surface evaluation at sample points
   - Check edge continuity

2. **Topological Validity**:
   - Verify all edges are connected
   - Check face orientations are consistent
   - Validate shell closure

---

## Implementation Phases

### Phase 1: Basic Conversion (Week 1)
- [ ] JSON parsing infrastructure
- [ ] Vertex conversion
- [ ] Basic edge conversion (straight edges only)
- [ ] Simple face conversion (single loop, planar)

### Phase 2: B-spline Support (Week 2)
- [ ] Surface B-spline conversion
- [ ] 2D curve B-spline conversion
- [ ] 2D to 3D curve mapping
- [ ] Edge with B-spline curves

### Phase 3: Complex Topology (Week 3)
- [ ] Multiple loops per face
- [ ] Shell conversion
- [ ] Body conversion (solid/compound)
- [ ] Orientation handling

### Phase 4: Polish and Testing (Week 4)
- [ ] Error handling
- [ ] Edge case handling
- [ ] Performance optimization
- [ ] Comprehensive testing

---

## Code Structure

```
ezdesign/
├── src/
│   ├── conversion/
│   │   ├── ezdesign_to_occt_converter.h
│   │   ├── ezdesign_to_occt_converter.cpp
│   │   ├── geometry_converter.h
│   │   ├── geometry_converter.cpp
│   │   ├── topology_converter.h
│   │   └── topology_converter.cpp
│   ├── data/
│   │   ├── ezdesign_types.h      // C++ structs matching JSON
│   │   └── json_parser.h         // JSON parsing
│   └── step_export/
│       ├── step_exporter.h
│       └── step_exporter.cpp
└── tests/
    ├── test_geometry_conversion.cpp
    ├── test_topology_conversion.cpp
    └── test_integration.cpp
```

---

## Dependencies

### Required OCCT Modules (from minimal build)
- `TKernel` - Core types
- `TKMath` - Mathematics
- `TKG2d`, `TKG3d` - 2D/3D geometry
- `TKGeomBase` - Geometry base classes
- `TKGeomAlgo` - Geometry algorithms
- `TKBRep` - Boundary representation
- `TKTopAlgo` - Topology algorithms
- `TKDESTEP` - STEP export

### Additional Libraries
- JSON parser (e.g., `nlohmann/json` or `rapidjson`)
- Standard C++ library

---

## Example Usage

```cpp
#include "conversion/ezdesign_to_occt_converter.h"
#include "step_export/step_exporter.h"

int main() {
    // 1. Load JSON data
    std::ifstream file("model.json");
    json data = json::parse(file);
    
    // 2. Parse ezdesign structure
    EzBody body = parseBody(data);
    
    // 3. Convert to OCCT
    EzDesignToOCCTConverter converter;
    TopoDS_Shape shape = converter.convertBody(body);
    
    // 4. Export to STEP
    StepExporter exporter;
    exporter.exportShape(shape, "output.step");
    
    return 0;
}
```

---

## References

- OCCT Documentation: `dox/user_guides/modeling_algos/`
- B-spline Mathematics: NURBS theory
- Half-Edge Data Structure: Computational geometry literature
- STEP File Format: ISO 10303 standard

---

## Questions and Open Issues

1. **Rational B-splines**: Are weights stored in JSON? If not, how to handle?
2. **Surface Trimming**: How are trimming curves stored in `trimming_loop`?
3. **Parameter Ranges**: Are `u_basis.bounds` and `v_basis.bounds` always needed?
4. **Edge Orientation**: How to determine correct edge orientation from half-edge?
5. **Performance**: What's the typical complexity of models? (number of faces, control points)

---

This proposal provides a comprehensive roadmap for converting ezdesign's topology/geometry to OCCT format. The bottom-up approach (Vertex → Edge → Wire → Face → Shell → Body) ensures dependencies are resolved correctly, and the detailed algorithms handle the B-spline geometry conversion accurately.

