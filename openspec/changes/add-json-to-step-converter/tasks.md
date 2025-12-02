## 1. Design and Planning (Estimated: 6-8 hours)

- [ ] 1.1 Analyze JSON format structure and topology hierarchy (2-3 hours)
  - Document complete JSON schema (Body, Shell, Face, Loop, HalfEdge, Edge, Vertex)
  - Map B-spline surface/curve data structures
  - Identify edge cases (degenerate edges, open loops, etc.)

- [ ] 1.2 Design conversion architecture (2-3 hours)
  - Define class structure for JSON reader and converter
  - Plan conversion pipeline: JSON → C++ structures → OCCT shapes → STEP
  - Design error handling and validation strategy

- [ ] 1.3 Review existing conversion proposal (2 hours)
  - Review `adm/cmake/EZDESIGN_TO_OCCT_CONVERSION_PROPOSAL.md`
  - Validate conversion algorithms against JSON structure
  - Identify any gaps or modifications needed

## 2. Implementation - JSON Parsing (Estimated: 8-10 hours)

- [ ] 2.1 Set up JSON parsing infrastructure (2-3 hours)
  - Add CMake detection for nlohmann/json (find_package, Homebrew paths)
  - Create JSON schema validation if needed
  - Implement basic JSON file reading

- [ ] 2.2 Implement topology element parsers (4-5 hours)
  - Parse Vertex objects (position data)
  - Parse Edge objects (half-edge references)
  - Parse HalfEdge objects (curve_data, navigation pointers)
  - Parse Loop objects (half-edge chains)
  - Parse Face objects (surface_data, loops)
  - Parse Shell objects (face collections)
  - Parse Body objects (shell collections)

- [ ] 2.3 Implement geometry element parsers (2 hours)
  - Parse B-spline surface data (control points, knot vectors, degrees)
  - Parse B-spline curve data (2D parametric curves)
  - Validate geometry data (dimensions, bounds, etc.)

## 3. Implementation - Geometry Conversion (Estimated: 12-16 hours)

- [ ] 3.1 Implement B-spline surface conversion (4-5 hours)
  - Reshape flattened control point arrays to 2D grid
  - Compute knot multiplicities from knot vectors
  - Create `Geom_BSplineSurface` objects
  - Handle rational vs non-rational surfaces

- [ ] 3.2 Implement B-spline curve conversion (3-4 hours)
  - Convert 2D parametric curves to `Geom2d_BSplineCurve`
  - Map 2D curves to 3D space via surface evaluation
  - Create `Geom_BSplineCurve` for edges
  - Handle parameter ranges and bounds

- [ ] 3.3 Implement knot multiplicity computation (2-3 hours)
  - Algorithm for computing multiplicities from knot vectors
  - Handle periodic vs non-periodic cases
  - Validate knot vector consistency

- [ ] 3.4 Implement control point reshaping utilities (2-3 hours)
  - Reshape 3D control points (flat array → 2D grid for surfaces)
  - Reshape 2D control points (flat array → 1D array for curves)
  - Handle dimension validation

## 4. Implementation - Topology Conversion (Estimated: 16-20 hours)

- [ ] 4.1 Implement vertex conversion (2 hours)
  - Convert JSON vertex to `TopoDS_Vertex`
  - Handle vertex caching to avoid duplicates

- [ ] 4.2 Implement half-edge to edge conversion (5-6 hours)
  - Traverse half-edge chain
  - Convert 2D parametric curves to 3D edges
  - Attach 2D curves to edges for face parameterization
  - Handle edge orientation

- [ ] 4.3 Implement loop to wire conversion (3-4 hours)
  - Traverse half-edge circular linked list
  - Build `TopoDS_Wire` from edge sequence
  - Handle wire closure validation
  - Set wire orientation (outer vs inner)

- [ ] 4.4 Implement face conversion (4-5 hours)
  - Convert surface to `Geom_BSplineSurface`
  - Create face from surface
  - Add outer loop (wire) as boundary
  - Add inner loops (wires) as holes
  - Respect ezdesign's CCW orientation convention (half-edges and their residing face normal follow right-hand rule)

- [ ] 4.5 Implement shell conversion (2 hours)
  - Convert all faces in shell
  - Build `TopoDS_Shell` from faces
  - Validate shell closure

- [ ] 4.6 Implement body conversion (2 hours)
  - Convert shells
  - Create `TopoDS_Solid` for single closed shell
  - Create `TopoDS_Compound` for multiple shells

## 5. Implementation - Command-Line Tool (Estimated: 6-8 hours)

- [ ] 5.1 Create `json2step` executable (2-3 hours)
  - Parse command-line arguments (input JSON, output STEP)
  - Integrate JSON reader and converter
  - Handle file I/O errors

- [ ] 5.2 Integrate STEP export (2 hours)
  - Use `STEPControl_Writer` to export converted shapes
  - Handle export errors and validation

- [ ] 5.3 Add error reporting and logging (2-3 hours)
  - Progress messages
  - Error messages with context
  - Validation warnings

## 6. Testing and Validation (Estimated: 8-10 hours)

- [ ] 6.1 Create unit tests for JSON parsing (2-3 hours)
  - Test each topology element parser
  - Test geometry element parsers
  - Test error handling for malformed JSON

- [ ] 6.2 Create unit tests for geometry conversion (2-3 hours)
  - Test B-spline surface conversion
  - Test B-spline curve conversion
  - Test knot multiplicity computation

- [ ] 6.3 Create unit tests for topology conversion (2-3 hours)
  - Test vertex/edge/wire/face/shell/body conversion
  - Test edge cases (degenerate edges, open loops)
  - Test orientation handling

- [ ] 6.4 Create integration tests (2 hours)
  - End-to-end test: JSON → STEP → validation
  - Test with sample JSON files
  - Verify STEP files can be read back by OCCT

## 7. Documentation (Estimated: 4-6 hours)

- [ ] 7.1 Write user documentation (2-3 hours)
  - Command-line tool usage
  - JSON format specification
  - Examples and use cases

- [ ] 7.2 Write API documentation (2-3 hours)
  - C++ API reference for converter classes
  - Code examples for programmatic use
  - Integration guide for external projects

## 8. Integration and Polish (Estimated: 4-6 hours)

- [ ] 8.1 Integrate into build system (2-3 hours)
  - Add to CMake build
  - Implement nlohmann/json detection (find_package, Homebrew paths)
  - Handle optional JSON library dependency gracefully
  - Update installation targets

- [ ] 8.2 Code review and refactoring (2-3 hours)
  - Review code quality
  - Optimize performance if needed
  - Fix any issues found

