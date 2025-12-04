// Created on: 2025
// Created by: OCCT json2step tool
// Copyright (c) 2025 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

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
#include <Geom_BSplineCurve.hxx>
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

  //! Check for errors
  Standard_EXPORT Standard_Boolean HasErrors() const;

  //! Get error messages
  Standard_EXPORT const std::vector<std::string>& GetErrors() const;

private:
  // Geometry conversion
  Handle(Geom_BSplineSurface) convertSurface(const EzSurfaceData& theData);
  Handle(Geom2d_BSplineCurve) convertCurve2D(const EzCurveData& theData);
  Handle(Geom_BSplineCurve) convertCurve3D(
    const Handle(Geom2d_BSplineCurve)& theCurve2d,
    const Handle(Geom_BSplineSurface)& theSurface,
    double theUMin,
    double theUMax);

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
  void computeKnotMultiplicities(
    const std::vector<double>& theKnots,
    int theDegree,
    TColStd_Array1OfInteger& theMultiplicities);
  TColgp_Array2OfPnt reshapeControlPoints3D(
    const std::vector<double>& theFlatData,
    int theNumU,
    int theNumV);
  TColgp_Array1OfPnt2d reshapeControlPoints2D(
    const std::vector<double>& theFlatData,
    int theNumPoints);
  int computeSampleCount(const Handle(Geom2d_BSplineCurve)& theCurve2d);

  // Accessors for topology elements
  const EzVertex& getVertex(int theId) const;
  const EzEdge& getEdge(int theId) const;
  const EzHalfEdge& getHalfEdge(int theId) const;
  const EzLoop& getLoop(int theId) const;
  const EzFace& getFace(int theId) const;
  const EzShell& getShell(int theId) const;
  const EzHalfEdge& getNextHalfEdge(int theId) const;

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

