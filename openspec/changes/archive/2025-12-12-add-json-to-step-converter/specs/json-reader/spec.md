# JSON Reader Capability

## ADDED Requirements

### Requirement: JSON File Reading

The system SHALL read JSON files containing topology/geometry data in ezdesign format.

#### Scenario: Read valid JSON file
- **GIVEN** a valid JSON file containing ezdesign topology/geometry data
- **WHEN** the JSON reader reads the file
- **THEN** the file is successfully parsed
- **AND** all topology elements (Body, Shell, Face, Loop, HalfEdge, Edge, Vertex) are extracted
- **AND** all geometry elements (B-spline surfaces and curves) are extracted
- **AND** the data is stored in C++ data structures

#### Scenario: Handle malformed JSON
- **GIVEN** a JSON file with syntax errors
- **WHEN** the JSON reader attempts to read the file
- **THEN** an error is reported with the line number and description
- **AND** no data is loaded

#### Scenario: Handle missing required fields
- **GIVEN** a JSON file missing required topology fields (e.g., missing `type` field)
- **WHEN** the JSON reader parses the file
- **THEN** an error is reported identifying the missing field
- **AND** parsing stops with clear error message

### Requirement: Topology Element Parsing

The system SHALL parse all topology elements from JSON format.

#### Scenario: Parse vertex elements
- **GIVEN** JSON containing vertex objects with `id`, `position`, and `half_edge` fields
- **WHEN** the parser processes vertices
- **THEN** each vertex is parsed with its 3D position coordinates
- **AND** vertex IDs are stored for topology linking

#### Scenario: Parse edge and half-edge elements
- **GIVEN** JSON containing edge and half-edge objects with navigation pointers (`next`, `previous`, `opposite`)
- **WHEN** the parser processes edges and half-edges
- **THEN** each half-edge is parsed with its curve data (if present)
- **AND** navigation pointers are stored for topology traversal
- **AND** edge-half-edge relationships are established

#### Scenario: Parse loop elements
- **GIVEN** JSON containing loop objects with `half_edge` pointer
- **WHEN** the parser processes loops
- **THEN** each loop is parsed with its starting half-edge reference
- **AND** loop-face relationships are established

#### Scenario: Parse face elements
- **GIVEN** JSON containing face objects with `surface_data` and `loops` arrays
- **WHEN** the parser processes faces
- **THEN** each face is parsed with its B-spline surface data
- **AND** all loops (outer and inner) are associated with the face
- **AND** the `is_surface_normal_same` flag is stored for orientation handling

#### Scenario: Parse shell and body elements
- **GIVEN** JSON containing shell and body objects with face/shell collections
- **WHEN** the parser processes shells and bodies
- **THEN** each shell is parsed with its face collection
- **AND** each body is parsed with its shell collection
- **AND** hierarchy relationships are established

### Requirement: Geometry Element Parsing

The system SHALL parse B-spline surface and curve geometry from JSON format.

#### Scenario: Parse B-spline surface data
- **GIVEN** JSON containing face `surface_data` with control points, knot vectors, and degrees
- **WHEN** the parser processes surface data
- **THEN** control points are extracted from flattened array
- **AND** U and V direction knot vectors are extracted
- **AND** U and V degrees are extracted
- **AND** surface bounds and rational flag are stored

#### Scenario: Parse B-spline curve data
- **GIVEN** JSON containing half-edge `curve_data` with 2D control points and knot vector
- **WHEN** the parser processes curve data
- **THEN** 2D control points (u, v parametric coordinates) are extracted
- **AND** knot vector and degree are extracted
- **AND** parameter bounds are stored

#### Scenario: Validate geometry data
- **GIVEN** JSON with geometry data
- **WHEN** the parser processes geometry
- **THEN** control point dimensions are validated (2 for curves, 3 for surfaces)
- **AND** knot vector lengths are validated against degrees
- **AND** control point counts match expected grid sizes

### Requirement: JSON to OCCT Shape Conversion

The system SHALL convert parsed JSON topology/geometry to OCCT `TopoDS_Shape` format.

#### Scenario: Convert vertex to TopoDS_Vertex
- **GIVEN** parsed vertex data with 3D position
- **WHEN** the converter processes the vertex
- **THEN** a `TopoDS_Vertex` is created at the specified position
- **AND** the vertex is cached to avoid duplicates

#### Scenario: Convert half-edge to TopoDS_Edge
- **GIVEN** parsed half-edge with 2D parametric curve and associated face surface
- **WHEN** the converter processes the half-edge
- **THEN** the 2D curve is converted to `Geom2d_BSplineCurve`
- **AND** the 2D curve is evaluated on the surface to create 3D `Geom_BSplineCurve`
- **AND** a `TopoDS_Edge` is created with the 3D curve
- **AND** the 2D curve is attached to the edge for face parameterization

#### Scenario: Convert loop to TopoDS_Wire
- **GIVEN** parsed loop with half-edge chain
- **WHEN** the converter processes the loop
- **THEN** the half-edge chain is traversed using `next` pointers
- **AND** each half-edge is converted to `TopoDS_Edge`
- **AND** edges are added to form a `TopoDS_Wire`
- **AND** wire closure is validated

#### Scenario: Convert face to TopoDS_Face
- **GIVEN** parsed face with B-spline surface and loops
- **WHEN** the converter processes the face
- **THEN** the surface is converted to `Geom_BSplineSurface`
- **AND** a `TopoDS_Face` is created from the surface
- **AND** the first loop is added as outer boundary (wire)
- **AND** subsequent loops are added as inner boundaries (holes)
- **AND** face orientation respects ezdesign's CCW convention (half-edges follow right-hand rule)

#### Scenario: Convert shell to TopoDS_Shell
- **GIVEN** parsed shell with face collection
- **WHEN** the converter processes the shell
- **THEN** all faces are converted to `TopoDS_Face`
- **AND** faces are added to form a `TopoDS_Shell`
- **AND** shell closure is checked

#### Scenario: Convert body to TopoDS_Solid or TopoDS_Compound
- **GIVEN** parsed body with shell collection
- **WHEN** the converter processes the body
- **THEN** if body has single closed shell, a `TopoDS_Solid` is created
- **AND** if body has multiple shells or open shells, a `TopoDS_Compound` is created
- **AND** all shells are added to the result

### Requirement: B-spline Geometry Conversion

The system SHALL convert B-spline surface and curve data from JSON format to OCCT geometry objects.

#### Scenario: Convert B-spline surface
- **GIVEN** surface data with control points, knot vectors, and degrees
- **WHEN** the converter processes the surface
- **THEN** control points are reshaped from flat array to 2D grid (`TColgp_Array2OfPnt`)
- **AND** knot multiplicities are computed from knot vectors
- **AND** a `Geom_BSplineSurface` is created with correct parameters
- **AND** rational vs non-rational cases are handled

#### Scenario: Convert 2D B-spline curve
- **GIVEN** curve data with 2D control points (u, v parametric coordinates) and knot vector
- **WHEN** the converter processes the curve
- **THEN** control points are reshaped to `TColgp_Array1OfPnt2d`
- **AND** knot multiplicities are computed
- **AND** a `Geom2d_BSplineCurve` is created

#### Scenario: Map 2D curve to 3D space
- **GIVEN** a 2D parametric curve and associated B-spline surface
- **WHEN** the converter creates 3D edge geometry
- **THEN** the 2D curve is sampled at multiple parameter values
- **AND** each sample point is evaluated on the surface: `surface->Value(u, v)`
- **AND** a 3D `Geom_BSplineCurve` is created from sampled 3D points
- **AND** the 3D curve is used for edge geometry

#### Scenario: Compute knot multiplicities
- **GIVEN** knot vector and degree for B-spline
- **WHEN** multiplicities are computed
- **THEN** for non-periodic B-splines, first and last multiplicities equal degree + 1
- **AND** internal multiplicities are computed from knot repetitions
- **AND** multiplicities are validated (1 <= mult <= degree)

### Requirement: Command-Line Tool

The system SHALL provide a command-line tool `json2step` for converting JSON files to STEP format.

#### Scenario: Convert JSON to STEP file
- **GIVEN** a valid JSON file and output STEP filename
- **WHEN** `json2step input.json output.step` is executed
- **THEN** the JSON file is read and parsed
- **AND** topology/geometry is converted to OCCT shapes
- **AND** shapes are exported to STEP format
- **AND** the STEP file is created at the specified path
- **AND** the generated STEP file is verified using OCCT's `STEPControl_Reader`
- **AND** if verification fails, detailed error messages identify the problematic entities or structure

#### Scenario: Verify generated STEP file
- **GIVEN** a STEP file generated by `json2step`
- **WHEN** OCCT's `STEPControl_Reader::ReadFile()` is called on the file
- **THEN** the file is successfully parsed (status = `IFSelect_RetDone`)
- **AND** check messages from `PrintCheckLoad()` are reported
- **AND** if parsing fails, detailed error messages identify:
  - Which entities have errors
  - What type of error occurred (syntax, structure, geometry)
  - Line numbers or entity numbers where errors occur
- **AND** the shape can be transferred and analyzed (faces, edges, vertices counted)

#### Scenario: Handle file errors
- **GIVEN** a non-existent input file
- **WHEN** `json2step` is executed
- **THEN** an error message is displayed
- **AND** the program exits with non-zero exit code

#### Scenario: Report conversion progress
- **GIVEN** a large JSON file being converted
- **WHEN** `json2step` processes the file
- **THEN** progress messages are displayed (e.g., "Reading JSON...", "Converting...", "Writing STEP...")
- **AND** error messages include context (which element failed)

### Requirement: Error Handling and Validation

The system SHALL validate JSON structure and geometry data, and provide clear error messages.

#### Scenario: Validate topology structure
- **GIVEN** JSON with invalid topology (e.g., loop not closed, missing opposite half-edge)
- **WHEN** the converter processes the topology
- **THEN** validation errors are detected
- **AND** clear error messages identify the issue and affected element

#### Scenario: Validate geometry data
- **GIVEN** JSON with invalid geometry (e.g., mismatched control point counts, invalid knot vectors)
- **WHEN** the converter processes the geometry
- **THEN** validation errors are detected
- **AND** error messages specify the issue (e.g., "Control point count mismatch: expected 16, got 15")

#### Scenario: Handle degenerate cases
- **GIVEN** JSON with degenerate geometry (e.g., edge with same start/end vertex)
- **WHEN** the converter processes the geometry
- **THEN** degenerate cases are detected
- **AND** appropriate handling is applied (create degenerate edge or skip with warning)

### Requirement: C++ API

The system SHALL provide C++ API classes for programmatic JSON reading and conversion.

#### Scenario: Use JSON reader API
- **GIVEN** C++ code that needs to read JSON topology/geometry
- **WHEN** `EzDesignJsonReader` is instantiated and `ReadFile()` is called
- **THEN** the JSON file is parsed
- **AND** topology/geometry data is accessible via getter methods

#### Scenario: Use converter API
- **GIVEN** parsed JSON data structures
- **WHEN** `EzDesignToOCCTConverter` is instantiated and `ConvertBody()` is called
- **THEN** the body is converted to `TopoDS_Shape`
- **AND** the shape can be used with OCCT APIs (e.g., `STEPControl_Writer`)

#### Scenario: Integrate with existing OCCT code
- **GIVEN** existing OCCT application code
- **WHEN** JSON reader and converter classes are used
- **THEN** converted `TopoDS_Shape` objects work with existing OCCT APIs
- **AND** shapes can be exported to STEP, IGES, or other formats
- **AND** shapes can be used in modeling operations

### Requirement: STEP File Verification

The system SHALL verify that generated STEP files are valid and can be read back by OCCT's STEP reader.

#### Scenario: Verify STEP file can be read
- **GIVEN** a STEP file generated by `json2step`
- **WHEN** `STEPControl_Reader::ReadFile()` is called
- **THEN** the file is successfully parsed (returns `IFSelect_RetDone`)
- **AND** no fatal errors are reported in check messages
- **AND** the shape can be transferred and analyzed

#### Scenario: Report STEP file errors
- **GIVEN** a STEP file with errors (syntax, structure, or geometry issues)
- **WHEN** `STEPControl_Reader::ReadFile()` is called
- **THEN** the read status indicates failure (`IFSelect_RetError` or `IFSelect_RetFail`)
- **AND** `PrintCheckLoad()` reports detailed error messages
- **AND** error messages identify:
  - Entity numbers or types with errors
  - Error categories (syntax, missing references, invalid geometry)
  - Specific issues (e.g., "Entity #123: Missing required attribute X")
- **AND** the converter can use this information to fix the STEP generation

#### Scenario: Validate STEP file structure
- **GIVEN** a generated STEP file
- **WHEN** verification is performed
- **THEN** the file has valid ISO-10303-21 header and structure
- **AND** all entity references are valid (no dangling references)
- **AND** geometry entities (surfaces, curves) are properly defined
- **AND** topology entities (faces, edges, vertices) are properly connected

