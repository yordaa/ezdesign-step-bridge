## Why

Users need to convert topology/geometry data stored in JSON format (ezdesign format) to STEP files for CAD data exchange. Currently, there is no built-in support in OCCT for reading JSON-based topology/geometry formats and converting them to STEP. Users must implement custom conversion code, which is error-prone and requires deep understanding of both the JSON format structure and OCCT's topology/geometry APIs.

## What Changes

- **ADDED**: JSON reader utility that parses ezdesign JSON format (Body → Shell → Face → Loop → HalfEdge → Edge → Vertex hierarchy)
- **ADDED**: Conversion functions to transform JSON topology/geometry to OCCT `TopoDS_Shape` format
- **ADDED**: B-spline surface and curve conversion from JSON control points and knot vectors to OCCT `Geom_BSplineSurface` and `Geom2d_BSplineCurve`
- **ADDED**: Command-line utility `json2step` that reads JSON file and exports to STEP format
- **ADDED**: C++ API classes for JSON parsing and conversion (`EzDesignJsonReader`, `EzDesignToOCCTConverter`)
- **ADDED**: Support for half-edge topology traversal and conversion to OCCT wires/faces
- **ADDED**: Error handling and validation for JSON structure and geometry data
- **ADDED**: Documentation and examples for using the JSON-to-STEP converter

## Impact

- **Affected specs**: `json-reader` capability (new)
- **Affected code**:
  - New toolkit: `TKJsonReader` (or utility in existing toolkit)
  - New command-line tool: `json2step` executable
  - Conversion utilities in `src/` or `tools/`
- **Breaking changes**: None (additive feature)
- **Dependencies**: 
  - JSON parsing library (nlohmann/json, available via package managers)
  - Existing OCCT modules: `TKDESTEP`, `TKBRep`, `TKGeomBase`, `TKGeomAlgo`
- **Platforms**: All supported platforms (Windows, Linux, macOS)

