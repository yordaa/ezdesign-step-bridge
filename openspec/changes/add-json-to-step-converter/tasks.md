## 1. Design and Planning (Estimated: 6-8 hours)

- [x] 1.1 Analyze JSON format structure and topology hierarchy (2-3 hours)
  - Document complete JSON schema (Body, Shell, Face, Loop, HalfEdge, Edge, Vertex)
  - Map B-spline surface/curve data structures
  - Identify edge cases (degenerate edges, open loops, etc.)

- [x] 1.2 Design conversion architecture (2-3 hours)
  - Define class structure for JSON reader and converter
  - Plan conversion pipeline: JSON → C++ structures → OCCT shapes → STEP
  - Design error handling and validation strategy

- [x] 1.3 Review existing conversion proposal (2 hours)
  - Review `adm/cmake/EZDESIGN_TO_OCCT_CONVERSION_PROPOSAL.md`
  - Validate conversion algorithms against JSON structure
  - Identify any gaps or modifications needed

## 2. Implementation - JSON Parsing (Estimated: 8-10 hours)

- [x] 2.1 Set up JSON parsing infrastructure (2-3 hours)
  - Add CMake detection for nlohmann/json (find_package, Homebrew paths)
  - Create JSON schema validation if needed
  - Implement basic JSON file reading

- [x] 2.2 Implement topology element parsers (4-5 hours)
  - Parse Vertex objects (position data)
  - Parse Edge objects (half-edge references)
  - Parse HalfEdge objects (curve_data, navigation pointers)
  - Parse Loop objects (half-edge chains)
  - Parse Face objects (surface_data, loops)
  - Parse Shell objects (face collections)
  - Parse Body objects (shell collections)

- [x] 2.3 Implement geometry element parsers (2 hours)
  - Parse B-spline surface data (control points, knot vectors, degrees)
  - Parse B-spline curve data (2D parametric curves)
  - Validate geometry data (dimensions, bounds, etc.)

## 3. Implementation - Geometry Conversion (Estimated: 12-16 hours)

- [x] 3.1 Implement B-spline surface conversion (4-5 hours)
  - Reshape flattened control point arrays to 2D grid
  - Compute knot multiplicities from knot vectors
  - Create `Geom_BSplineSurface` objects
  - Handle rational vs non-rational surfaces

- [x] 3.2 Implement B-spline curve conversion (3-4 hours)
  - Convert 2D parametric curves to `Geom2d_BSplineCurve`
  - Map 2D curves to 3D space via surface evaluation
  - Create `Geom_BSplineCurve` for edges
  - Handle parameter ranges and bounds

- [x] 3.3 Implement knot multiplicity computation (2-3 hours)
  - Algorithm for computing multiplicities from knot vectors
  - Handle periodic vs non-periodic cases
  - Validate knot vector consistency

- [x] 3.4 Implement control point reshaping utilities (2-3 hours)
  - Reshape 3D control points (flat array → 2D grid for surfaces)
  - Reshape 2D control points (flat array → 1D array for curves)
  - Handle dimension validation

## 4. Implementation - Topology Conversion (Estimated: 16-20 hours)

- [x] 4.1 Implement vertex conversion (2 hours)
  - Convert JSON vertex to `TopoDS_Vertex`
  - Handle vertex caching to avoid duplicates

- [x] 4.2 Implement half-edge to edge conversion (5-6 hours)
  - Traverse half-edge chain
  - Convert 2D parametric curves to 3D edges
  - Attach 2D curves to edges for face parameterization
  - Handle edge orientation

- [x] 4.3 Implement loop to wire conversion (3-4 hours)
  - Traverse half-edge circular linked list
  - Build `TopoDS_Wire` from edge sequence
  - Handle wire closure validation
  - Set wire orientation (outer vs inner)

- [x] 4.4 Implement face conversion (4-5 hours)
  - Convert surface to `Geom_BSplineSurface`
  - Create face from surface
  - Add outer loop (wire) as boundary
  - Add inner loops (wires) as holes
  - Respect ezdesign's CCW orientation convention (half-edges and their residing face normal follow right-hand rule)

- [x] 4.5 Implement shell conversion (2 hours)
  - Convert all faces in shell
  - Build `TopoDS_Shell` from faces
  - Validate shell closure

- [x] 4.6 Implement body conversion (2 hours)
  - Convert shells
  - Create `TopoDS_Solid` for single closed shell
  - Create `TopoDS_Compound` for multiple shells

## 5. Implementation - Command-Line Tool (Estimated: 6-8 hours)

- [x] 5.1 Create `json2step` executable (2-3 hours)
  - Parse command-line arguments (input JSON, output STEP)
  - Integrate JSON reader and converter
  - Handle file I/O errors

- [x] 5.2 Integrate STEP export (2 hours)
  - Use `STEPControl_Writer` to export converted shapes
  - Handle export errors and validation

- [x] 5.3 Add error reporting and logging (2-3 hours)
  - Progress messages
  - Error messages with context
  - Validation warnings

## 6. Testing and Validation (Estimated: 8-10 hours)

- [ ] 6.1 Build and verify executable (next step)
  - [ ] 6.1.1 Complete compilation fixes
    - [ ] All tasks in 8.2 must be completed first
  - [ ] 6.1.2 Verify build success
    - [ ] Run `cmake --build . --target json2step` successfully
    - [ ] Locate built executable (check build/bin or build/tools/json2step)
    - [ ] Verify executable has correct permissions
  - [ ] 6.1.3 Test basic execution
    - [ ] Run executable with no arguments (should show usage)
    - [ ] Run executable with wrong number of arguments (should show usage)
    - [ ] Verify error messages are clear

- [ ] 6.2 Integration test with real data (next step)
  - [ ] 6.2.1 Prepare test data
    - [ ] Verify `/Users/songyang/Downloads/a.txt` exists and is readable
    - [ ] Check JSON file structure matches expected format
    - [ ] Note any potential issues in the JSON structure
  - [ ] 6.2.2 Test JSON parsing
    - [ ] Run: `json2step /Users/songyang/Downloads/a.txt test_output.step`
    - [ ] Verify JSON parsing completes without errors
    - [ ] Check for any parsing warnings or errors in output
    - [ ] Verify all topology elements are parsed correctly
  - [ ] 6.2.3 Test conversion to OCCT shapes
    - [ ] Verify conversion to OCCT shapes succeeds
    - [ ] Check for conversion warnings or errors
    - [ ] Verify shape hierarchy is correct (Body → Shell → Face → Loop → Edge → Vertex)
  - [ ] 6.2.4 Test STEP file generation
    - [ ] Verify STEP file is generated
    - [ ] Check file size is reasonable (not empty, not suspiciously small)
    - [ ] Verify STEP file has correct extension and format
  - [ ] 6.2.5 Verify STEP file can be read back
    - [ ] Use OCCT's STEPControl_Reader to read generated STEP file
    - [ ] Verify shape can be loaded without errors
    - [ ] Compare basic properties (number of faces, edges, vertices)
    - [ ] Optionally visualize in DRAW or other OCCT tool

- [ ] 6.3 Create unit tests for JSON parsing (2-3 hours)
  - [ ] 6.3.1 Test topology element parsers
    - [ ] Test parseVertex with valid and invalid data
    - [ ] Test parseEdge with valid and invalid data
    - [ ] Test parseHalfEdge with curve_data and without
    - [ ] Test parseLoop with closed and open loops
    - [ ] Test parseFace with single and multiple loops
    - [ ] Test parseShell with single and multiple faces
    - [ ] Test parseBody with single and multiple shells
  - [ ] 6.3.2 Test geometry element parsers
    - [ ] Test B-spline surface parsing (control points, knots, degrees)
    - [ ] Test B-spline curve parsing (2D parametric)
    - [ ] Test rational vs non-rational surfaces
    - [ ] Test validation of geometry data dimensions
  - [ ] 6.3.3 Test error handling
    - [ ] Test malformed JSON (missing fields, wrong types)
    - [ ] Test invalid topology references (non-existent IDs)
    - [ ] Test invalid geometry data (wrong dimensions, invalid knots)

- [ ] 6.4 Create unit tests for geometry conversion (2-3 hours)
  - [ ] 6.4.1 Test B-spline surface conversion
    - [ ] Test control point reshaping (flat array → 2D grid)
    - [ ] Test knot multiplicity computation for surfaces
    - [ ] Test rational vs non-rational surface creation
  - [ ] 6.4.2 Test B-spline curve conversion
    - [ ] Test 2D curve conversion (Geom2d_BSplineCurve)
    - [ ] Test 3D curve conversion via surface evaluation
    - [ ] Test parameter range handling
  - [ ] 6.4.3 Test knot multiplicity computation
    - [ ] Test with unique knots (multiplicity = 1 for internal)
    - [ ] Test with repeated knots
    - [ ] Test first/last knot multiplicities (degree + 1)
    - [ ] Test edge cases (insufficient knots, invalid degree)

- [ ] 6.5 Create unit tests for topology conversion (2-3 hours)
  - [ ] 6.5.1 Test basic conversions
    - [ ] Test vertex conversion and caching
    - [ ] Test edge conversion from half-edges
    - [ ] Test wire conversion from loops
    - [ ] Test face conversion with outer and inner loops
    - [ ] Test shell conversion
    - [ ] Test body conversion (solid vs compound)
  - [ ] 6.5.2 Test edge cases
    - [ ] Test degenerate edges (same start/end vertex)
    - [ ] Test open loops (should fail gracefully)
    - [ ] Test face with no loops (use surface domain)
  - [ ] 6.5.3 Test orientation handling
    - [ ] Test CCW orientation convention
    - [ ] Test is_surface_normal_same flag handling
    - [ ] Verify right-hand rule is respected

- [ ] 6.6 Create additional integration tests (2 hours)
  - [ ] 6.6.1 Test with minimal test cases
    - [ ] Create simple box JSON (single face, single loop)
    - [ ] Create cylinder JSON (multiple faces, closed shell)
    - [ ] Verify both convert successfully to STEP
  - [ ] 6.6.2 Test edge cases
    - [ ] Test degenerate edges (should handle or fail gracefully)
    - [ ] Test open loops (should fail with clear error)
    - [ ] Test invalid topology (missing references)
  - [ ] 6.6.3 Verify STEP file round-trip
    - [ ] Generate STEP from JSON
    - [ ] Read STEP back with OCCT
    - [ ] Compare basic shape properties
    - [ ] Optionally export back to JSON and compare

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

- [x] 8.1 Integrate into build system (2-3 hours)
  - Add to CMake build
  - Implement nlohmann/json detection (find_package, Homebrew paths)
  - Handle optional JSON library dependency gracefully
  - Update installation targets

- [ ] 8.2 Fix compilation issues (in progress)
  - [ ] 8.2.1 Fix nlohmann/json include path
    - [ ] Ensure CMakeLists.txt properly sets include directories after target creation
    - [ ] Verify nlohmann/json.hpp can be found by compiler
    - [ ] Test that include path works in both header and source files
  - [ ] 8.2.2 Fix GeomAPI_Interpolate usage
    - [ ] Change TColgp_Array1OfPnt to Handle(TColgp_HArray1OfPnt) in convertCurve3D
    - [ ] Change TColStd_Array1OfReal to Handle(TColStd_HArray1OfReal) for parameters
    - [ ] Fix Perform() call (remove parameters argument)
    - [ ] Fix fallback case to use Array1OfPnt instead of Handle type
  - [ ] 8.2.3 Fix forward declaration issues
    - [ ] Replace incorrect forward declaration in EzDesignJsonReader.hxx
    - [ ] Either include nlohmann/json.hpp in header or use proper forward declaration
    - [ ] Ensure json type is available where needed
  - [ ] 8.2.4 Resolve remaining compiler errors
    - [ ] Fix unused parameter warnings
    - [ ] Verify all includes are correct
    - [ ] Ensure all OCCT types are properly included
  - [ ] 8.2.5 Verify successful build
    - [ ] Build json2step target without errors
    - [ ] Verify executable is created in correct location
    - [ ] Test executable runs and shows usage message

- [ ] 8.3 Code review and refactoring (2-3 hours)
  - Review code quality
  - Optimize performance if needed
  - Fix any issues found

