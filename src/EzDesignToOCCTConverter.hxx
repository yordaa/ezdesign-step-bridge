// Created on: 2025
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#ifndef _EzDesignToOCCTConverter_HeaderFile
#define _EzDesignToOCCTConverter_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shell.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include "EzDesignTypes.hxx"
#include "EzDesignJsonReader.hxx"
#include <vector>
#include <string>
#include <map>

//! Converter from ezdesign JSON data structures to OCCT TopoDS_Shape format
class EzDesignToOCCTConverter
{
public:
  DEFINE_STANDARD_ALLOC

  //! Constructor
  Standard_EXPORT EzDesignToOCCTConverter(const EzDesignJsonReader& theReader);

  //! Destructor
  Standard_EXPORT ~EzDesignToOCCTConverter();

  //! Convert body to TopoDS_Shape
  Standard_EXPORT TopoDS_Shape ConvertBody(const EzBody& theBody);

  //! Get error messages
  Standard_EXPORT const std::vector<std::string>& GetErrors() const;

private:
  // Geometry conversion
  Handle(Geom_BSplineSurface) convertSurface(const EzSurfaceData& theData);
  Handle(Geom2d_BSplineCurve) convertCurve2D(const EzCurveData& theData);

  // Topology conversion
  TopoDS_Vertex convertVertex(const EzVertex& theVertex);
  TopoDS_Edge convertHalfEdge(
    const EzHalfEdge& theHalfEdge,
    const Handle(Geom_BSplineSurface)& theSurface,
    bool theIsSurfaceNormalSame);
  TopoDS_Wire convertLoop(
    const EzLoop& theLoop,
    const Handle(Geom_BSplineSurface)& theSurface,
    bool theIsSurfaceNormalSame);
  TopoDS_Face convertFace(const EzFace& theFace);
  TopoDS_Shell convertShell(const EzShell& theShell);
  TopoDS_Shape convertBody(const EzBody& theBody);

  // Helper methods
  bool buildKnotData(
    const std::vector<double>& theKnotSequence,
    int theDegree,
    int thePoleCount,
    std::vector<double>& theKnots,
    std::vector<int>& theMultiplicities);
  TColgp_Array2OfPnt reshapeControlPoints3D(
    const std::vector<double>& theFlatData,
    int theNumU,
    int theNumV);
  TColgp_Array1OfPnt2d reshapeControlPoints2D(
    const std::vector<double>& theFlatData,
    int theNumPoints);
  // Helper to add pcurve to existing edge (for edges shared between faces on different surfaces)
  void addPCurveToEdge(
    TopoDS_Edge& theEdge,
    const EzHalfEdge& theHalfEdge,
    const Handle(Geom_BSplineSurface)& theSurface);

  // Add error
  void addError(const std::string& theError);

private:
  const EzDesignJsonReader& myReader;
  std::map<int, TopoDS_Vertex> myVertexMap;  // Map: JSON vertex_id → OCCT TopoDS_Vertex
  std::map<int, TopoDS_Edge> myEdgeMap;      // Map: JSON edge_id → OCCT TopoDS_Edge
  std::vector<std::string> myErrors;
};

#endif // _EzDesignToOCCTConverter_HeaderFile
