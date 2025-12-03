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

#include "EzDesignToOCCTConverter.hxx"

#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <TopoDS.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRep_Tool.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Precision.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_HArray1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom2dConvert.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Dir2d.hxx>
#include <sstream>

//=======================================================================
// function : EzDesignToOCCTConverter
// purpose  : Constructor
//=======================================================================
EzDesignToOCCTConverter::EzDesignToOCCTConverter(const EzDesignJsonReader& theReader)
: myReader(theReader)
{
}

//=======================================================================
// function : ~EzDesignToOCCTConverter
// purpose  : Destructor
//=======================================================================
EzDesignToOCCTConverter::~EzDesignToOCCTConverter()
{
}

//=======================================================================
// function : ConvertBody
// purpose  : Convert body to TopoDS_Shape
//=======================================================================
TopoDS_Shape EzDesignToOCCTConverter::ConvertBody(const EzBody& theBody)
{
  myErrors.clear();
  myVertexCache.clear();

  try {
    OCC_CATCH_SIGNALS
    return convertBody(theBody);
  }
  catch (const Standard_Failure& e) {
    addError(std::string("OCCT exception: ") + e.GetMessageString());
    return TopoDS_Shape();
  }
  catch (const std::exception& e) {
    addError(std::string("Standard exception: ") + e.what());
    return TopoDS_Shape();
  }
  catch (...) {
    addError("Unknown exception occurred");
    return TopoDS_Shape();
  }
}

//=======================================================================
// function : convertBody
// purpose  : Convert body to TopoDS_Shape
//=======================================================================
TopoDS_Shape EzDesignToOCCTConverter::convertBody(const EzBody& theBody)
{
  if (theBody.shell_ids.empty()) {
    addError("Body has no shells");
    return TopoDS_Shape();
  }

  if (theBody.shell_ids.size() == 1) {
    // Single shell - try to make solid
    const EzShell& shell = getShell(theBody.shell_ids[0]);
    TopoDS_Shell occtShell = convertShell(shell);

    if (occtShell.IsNull()) {
      return TopoDS_Shape();
    }

    if (BRep_Tool::IsClosed(occtShell)) {
      // Closed shell → Solid
      BRepBuilderAPI_MakeSolid solidMaker(occtShell);
      if (solidMaker.IsDone()) {
        return TopoDS::Solid(solidMaker.Solid());
      }
    }

    // If solid creation fails, return shell
    return occtShell;
  }
  else {
    // Multiple shells → Compound
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);

    for (int shellId : theBody.shell_ids) {
      const EzShell& shell = getShell(shellId);
      TopoDS_Shell occtShell = convertShell(shell);
      if (!occtShell.IsNull()) {
        builder.Add(compound, occtShell);
      }
    }

    return compound;
  }
}

//=======================================================================
// function : convertShell
// purpose  : Convert shell to TopoDS_Shell
//=======================================================================
TopoDS_Shell EzDesignToOCCTConverter::convertShell(const EzShell& theShell)
{
  BRep_Builder builder;
  TopoDS_Shell resultShell;
  builder.MakeShell(resultShell);

  // Convert and add all faces
  for (int faceId : theShell.face_ids) {
    const EzFace& face = getFace(faceId);
    TopoDS_Face occtFace = convertFace(face);

    if (!occtFace.IsNull()) {
      builder.Add(resultShell, occtFace);
    }
  }

  return resultShell;
}

//=======================================================================
// function : convertFace
// purpose  : Convert face to TopoDS_Face
//=======================================================================
TopoDS_Face EzDesignToOCCTConverter::convertFace(const EzFace& theFace)
{
  // 1. Convert surface
  Handle(Geom_BSplineSurface) surface = convertSurface(theFace.surface_data);
  if (surface.IsNull()) {
    addError("Face " + std::to_string(theFace.id) + ": Failed to convert surface");
    return TopoDS_Face();
  }

  // 2. Convert loops (wires) first
  // Note: Half-edges follow ezdesign convention:
  // - CCW w.r.t. face outward normal (right-hand rule)
  // - If is_surface_normal_same=true: CCW in UV domain
  // - If is_surface_normal_same=false: CW in UV domain (but CCW for face)
  TopoDS_Wire outerWire;
  TopTools_ListOfShape innerWires;
  
  bool isFirstLoop = true;
  for (int loopId : theFace.loop_ids) {
    const EzLoop& loop = getLoop(loopId);
    TopoDS_Wire wire = convertLoop(loop, surface, theFace.is_surface_normal_same);

    if (wire.IsNull()) {
      addError("Face " + std::to_string(theFace.id) + ": Failed to convert loop " + std::to_string(loopId));
      continue;
    }

    if (isFirstLoop) {
      // First loop is outer boundary
      outerWire = wire;
      isFirstLoop = false;
    }
    else {
      // Subsequent loops are inner boundaries (holes)
      innerWires.Append(wire);
    }
  }

  if (outerWire.IsNull()) {
    addError("Face " + std::to_string(theFace.id) + ": No outer boundary loop found");
    return TopoDS_Face();
  }

  // 3. Create face from surface with outer wire
  // This ensures we only have the wires we explicitly added, not the surface's natural boundaries
  // Inside=true means the wire is the outer boundary (the face is inside the wire)
  BRepBuilderAPI_MakeFace faceMaker(surface, outerWire, Standard_True);
  
  // 4. Add inner wires (holes) if any
  TopTools_ListIteratorOfListOfShape innerIt(innerWires);
  for (; innerIt.More(); innerIt.Next()) {
    faceMaker.Add(TopoDS::Wire(innerIt.Value()));
  }

  if (!faceMaker.IsDone()) {
    addError("Face " + std::to_string(theFace.id) + ": Failed to create face");
    return TopoDS_Face();
  }

  TopoDS_Face resultFace = faceMaker.Face();
  return resultFace;
}

//=======================================================================
// function : convertLoop
// purpose  : Convert loop to TopoDS_Wire
//=======================================================================
TopoDS_Wire EzDesignToOCCTConverter::convertLoop(
  const EzLoop& theLoop,
  const Handle(Geom_BSplineSurface)& theSurface,
  bool theIsSurfaceNormalSame)
{
  BRepBuilderAPI_MakeWire wireMaker;

  // Traverse half-edge chain
  // ezdesign convention: half-edges are CCW w.r.t. face outward normal (right-hand rule)
  int currentHeId = theLoop.half_edge_id;
  int startHeId = currentHeId;
  int maxIterations = 1000;  // Safety limit
  int iterations = 0;

  do {
    if (iterations++ > maxIterations) {
      addError("Loop " + std::to_string(theLoop.id) + ": Maximum iterations reached (possible infinite loop)");
      return TopoDS_Wire();
    }

    const EzHalfEdge& halfEdge = getHalfEdge(currentHeId);
    if (halfEdge.id == 0) {
      addError("Loop " + std::to_string(theLoop.id) + ": Half-edge " + std::to_string(currentHeId) + " not found");
      return TopoDS_Wire();
    }

    TopoDS_Edge edge = convertHalfEdge(halfEdge, theSurface, theIsSurfaceNormalSame);
    if (edge.IsNull()) {
      addError("Loop " + std::to_string(theLoop.id) + ": Failed to convert half-edge " + std::to_string(currentHeId));
      return TopoDS_Wire();
    }

    wireMaker.Add(edge);

    // Move to next half-edge
    currentHeId = halfEdge.next_id;
  } while (currentHeId != startHeId && currentHeId != 0);

  if (currentHeId != startHeId) {
    addError("Loop " + std::to_string(theLoop.id) + ": Loop is not closed");
    return TopoDS_Wire();
  }

  if (!wireMaker.IsDone()) {
    addError("Loop " + std::to_string(theLoop.id) + ": Failed to create wire");
    return TopoDS_Wire();
  }

  TopoDS_Wire wire = wireMaker.Wire();
  return wire;
}

//=======================================================================
// function : convertHalfEdge
// purpose  : Convert half-edge to TopoDS_Edge
//=======================================================================
TopoDS_Edge EzDesignToOCCTConverter::convertHalfEdge(
  const EzHalfEdge& theHalfEdge,
  const Handle(Geom_BSplineSurface)& theSurface,
  bool /*theIsSurfaceNormalSame*/)
{
  // 0. Skip conceptual half-edges (loop_id == 0 means conceptual only, no real meaning)
  if (theHalfEdge.loop_id == 0) {
    // This is a conceptual half-edge, should not create real topology
    return TopoDS_Edge();
  }

  // 1. Get start and end vertices
  const EzVertex& startVertex = getVertex(theHalfEdge.vertex_id);
  if (startVertex.id == 0) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Start vertex " + std::to_string(theHalfEdge.vertex_id) + " not found");
    return TopoDS_Edge();
  }

  const EzHalfEdge& nextHalfEdge = getNextHalfEdge(theHalfEdge.id);
  if (nextHalfEdge.id == 0) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Next half-edge not found");
    return TopoDS_Edge();
  }

  const EzVertex& endVertex = getVertex(nextHalfEdge.vertex_id);
  if (endVertex.id == 0) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": End vertex not found");
    return TopoDS_Edge();
  }

  TopoDS_Vertex vStart = convertVertex(startVertex);
  TopoDS_Vertex vEnd = convertVertex(endVertex);

  // 2. Get curve_data - try current half-edge first, then opposite if available
  EzCurveData curveData = theHalfEdge.curve_data;
  bool hasCurveData = !curveData.control_points.data.empty() &&
                      curveData.control_points.number_u_points >= 2;

  // If no curve_data, try opposite half-edge
  if (!hasCurveData && theHalfEdge.opposite_id != 0) {
    const EzHalfEdge& oppositeHe = getHalfEdge(theHalfEdge.opposite_id);
    if (oppositeHe.id != 0 && !oppositeHe.curve_data.control_points.data.empty() &&
        oppositeHe.curve_data.control_points.number_u_points >= 2) {
      curveData = oppositeHe.curve_data;
      hasCurveData = true;
    }
  }

  // 3. If still no curve_data, we need to generate a pcurve for edges on surfaces
  // Create a straight 2D line in parametric space as fallback
  if (!hasCurveData) {
    // Generate a simple 2D line pcurve by projecting the 3D edge onto the surface
    gp_Pnt p1(startVertex.position[0], startVertex.position[1], startVertex.position[2]);
    gp_Pnt p2(endVertex.position[0], endVertex.position[1], endVertex.position[2]);
    
    // Project points onto surface to get parametric coordinates
    Standard_Real u1, v1, u2, v2;
    GeomAPI_ProjectPointOnSurf proj1(p1, theSurface);
    GeomAPI_ProjectPointOnSurf proj2(p2, theSurface);
    
    if (proj1.NbPoints() > 0 && proj2.NbPoints() > 0) {
      proj1.Parameters(1, u1, v1);
      proj2.Parameters(1, u2, v2);
      gp_Pnt2d uv1(u1, v1);
      gp_Pnt2d uv2(u2, v2);
      
      // Create a 2D line in parametric space
      gp_Vec2d dir(uv2.X() - uv1.X(), uv2.Y() - uv1.Y());
      Handle(Geom2d_Line) line2d = new Geom2d_Line(uv1, gp_Dir2d(dir));
      
      // Determine parameter range for the line (distance in parametric space)
      Standard_Real paramDist = dir.Magnitude();
      if (paramDist < Precision::Confusion()) {
        addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Degenerate parametric line");
        return TopoDS_Edge();
      }
      
      // Convert Geom2d_Line to Geom2d_BSplineCurve so we can use convertCurve3D
      Handle(Geom2d_TrimmedCurve) trimmedLine = new Geom2d_TrimmedCurve(line2d, 0.0, paramDist);
      Handle(Geom2d_BSplineCurve) line2dBSpline = Geom2dConvert::CurveToBSplineCurve(trimmedLine);
      
      if (line2dBSpline.IsNull()) {
        addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to convert 2D line to BSpline");
        return TopoDS_Edge();
      }
      
      // Evaluate the 2D curve on the surface to get the 3D curve
      Handle(Geom_BSplineCurve) curve3d = convertCurve3D(line2dBSpline, theSurface, 0.0, paramDist);
      if (curve3d.IsNull()) {
        addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to convert 2D line to 3D curve on surface");
        return TopoDS_Edge();
      }
      
      // Create edge from the 3D curve (which follows the surface)
      BRepBuilderAPI_MakeEdge edgeMaker(curve3d, vStart, vEnd);
      if (!edgeMaker.IsDone()) {
        addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to create edge from 3D curve");
        return TopoDS_Edge();
      }
      TopoDS_Edge edge = edgeMaker.Edge();
      
      // Attach the generated 2D curve to the edge
      BRep_Builder builder;
      TopLoc_Location identityLoc;
      builder.UpdateEdge(edge, line2dBSpline, theSurface, identityLoc, Precision::Confusion());
      
      return edge;
    }
    else {
      // Fallback: create straight edge without pcurve (should not happen for edges on surfaces)
      BRepBuilderAPI_MakeEdge edgeMaker(p1, p2);
      if (!edgeMaker.IsDone()) {
        addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to create straight edge");
        return TopoDS_Edge();
      }
      return edgeMaker.Edge();
    }
  }

  // 4. Convert 2D curve
  Handle(Geom2d_BSplineCurve) curve2d = convertCurve2D(curveData);
  if (curve2d.IsNull()) {
    // Fallback to straight edge if curve conversion fails
    gp_Pnt p1(startVertex.position[0], startVertex.position[1], startVertex.position[2]);
    gp_Pnt p2(endVertex.position[0], endVertex.position[1], endVertex.position[2]);
    BRepBuilderAPI_MakeEdge edgeMaker(p1, p2);
    if (!edgeMaker.IsDone()) {
      addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to create fallback straight edge");
      return TopoDS_Edge();
    }
    return edgeMaker.Edge();
  }

  // Note: ezdesign convention ensures half-edges are CCW w.r.t. face outward normal
  // The 2D curve orientation is already correct per this convention

  // 5. Get parameter range
  double uMin = curveData.basis.bounds.minimum;
  double uMax = curveData.basis.bounds.maximum;

  // 6. Create 3D curve by sampling 2D curve on surface
  Handle(Geom_BSplineCurve) curve3d = convertCurve3D(curve2d, theSurface, uMin, uMax);
  if (curve3d.IsNull()) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to convert 3D curve");
    return TopoDS_Edge();
  }

  // 7. Create edge from 3D curve
  BRepBuilderAPI_MakeEdge edgeMaker(curve3d, vStart, vEnd);
  if (!edgeMaker.IsDone()) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to create edge from 3D curve");
    return TopoDS_Edge();
  }

  TopoDS_Edge edge = edgeMaker.Edge();

  // 8. Attach 2D curve to edge for face parameterization
  // Note: The face will be created later, but we attach the 2D curve now
  // using the surface and an identity location
  // This ensures SURFACE_CURVE will have at least one pcurve in STEP export
  BRep_Builder builder;
  TopLoc_Location identityLoc;  // Identity location
  builder.UpdateEdge(edge, curve2d, theSurface, identityLoc, Precision::Confusion());

  return edge;
}

//=======================================================================
// function : convertVertex
// purpose  : Convert vertex to TopoDS_Vertex
//=======================================================================
TopoDS_Vertex EzDesignToOCCTConverter::convertVertex(const EzVertex& theVertex)
{
  // Check cache first
  auto it = myVertexCache.find(theVertex.id);
  if (it != myVertexCache.end()) {
    return it->second;
  }

  // Create new vertex
  gp_Pnt point(theVertex.position[0], theVertex.position[1], theVertex.position[2]);
  BRepBuilderAPI_MakeVertex vertexMaker(point);
  if (!vertexMaker.IsDone()) {
    addError("Vertex " + std::to_string(theVertex.id) + ": Failed to create vertex");
    return TopoDS_Vertex();
  }

  TopoDS_Vertex vertex = vertexMaker.Vertex();
  myVertexCache[theVertex.id] = vertex;
  return vertex;
}

//=======================================================================
// function : convertSurface
// purpose  : Convert B-spline surface
//=======================================================================
Handle(Geom_BSplineSurface) EzDesignToOCCTConverter::convertSurface(const EzSurfaceData& theData)
{
  const auto& cp = theData.control_points;
  const auto& uBasis = theData.u_basis;
  const auto& vBasis = theData.v_basis;

  // 1. Reshape control points from flat array to 2D array
  int numU = cp.number_u_points;
  int numV = cp.number_v_points;
  TColgp_Array2OfPnt poles = reshapeControlPoints3D(cp.data, numU, numV);

  // 2. Process U-direction knots and multiplicities
  // First, collapse repeated knots to unique knots
  // For non-periodic B-splines: numPoles = sum(multiplicities) - degree - 1
  // Expected: numU = sum(uMults) - uDegree - 1
  // So: sum(uMults) = numU + uDegree + 1
  
  // Collapse to unique knots first to determine array sizes
  const double tol = 1e-9;
  std::vector<double> uUniqueKnots;
  int i = 0;
  while (i < static_cast<int>(uBasis.knot_vector.size())) {
    double currentKnot = uBasis.knot_vector[i];
    uUniqueKnots.push_back(currentKnot);
    int j = i + 1;
    while (j < static_cast<int>(uBasis.knot_vector.size()) && 
           std::abs(uBasis.knot_vector[j] - currentKnot) < tol) {
      j++;
    }
    i = j;
  }
  
  int nUUniqueKnots = static_cast<int>(uUniqueKnots.size());
  int expectedUSum = numU + uBasis.degree + 1;
  
  TColStd_Array1OfReal uKnots(1, nUUniqueKnots);
  TColStd_Array1OfInteger uMults(1, nUUniqueKnots);
  computeKnotMultiplicities(uBasis.knot_vector, uBasis.degree, uMults);
  
  // Validate and adjust multiplicities to match expected sum
  // For non-periodic: numPoles = sum(multiplicities) - degree - 1
  // So: sum(multiplicities) = numPoles + degree + 1
  int actualUSum = 0;
  for (int i = 1; i <= nUUniqueKnots; i++) {
    actualUSum += uMults.Value(i);
  }
  
  if (actualUSum != expectedUSum) {
    // Adjust multiplicities to match expected sum
    int diff = expectedUSum - actualUSum;
    
    if (diff > 0) {
      // Need to increase multiplicities
      // Distribute excess to internal knots (up to degree limit)
      int internalKnots = nUUniqueKnots - 2;
      if (internalKnots > 0) {
        int perKnot = diff / internalKnots;
        int remainder = diff % internalKnots;
        for (int i = 2; i < nUUniqueKnots; i++) {
          int currentMult = uMults.Value(i);
          int newMult = currentMult + perKnot + (i - 2 < remainder ? 1 : 0);
          if (newMult > uBasis.degree) {
            newMult = uBasis.degree;
          }
          uMults.SetValue(i, newMult);
        }
      }
    }
    else {
      // Need to decrease multiplicities
      // Reduce internal knots first, then adjust first/last if needed
      int excess = -diff;
      for (int i = 2; i < nUUniqueKnots && excess > 0; i++) {
        int currentMult = uMults.Value(i);
        int reduction = (excess < currentMult - 1) ? excess : (currentMult - 1);
        uMults.SetValue(i, currentMult - reduction);
        excess -= reduction;
      }
      // If still excess, reduce first/last (but keep at least 1)
      if (excess > 0 && nUUniqueKnots > 0) {
        int firstMult = uMults.Value(1);
        if (firstMult > 1 && excess > 0) {
          int reduction = (excess < firstMult - 1) ? excess : (firstMult - 1);
          uMults.SetValue(1, firstMult - reduction);
          excess -= reduction;
        }
      }
      if (excess > 0 && nUUniqueKnots > 1) {
        int lastMult = uMults.Value(nUUniqueKnots);
        if (lastMult > 1 && excess > 0) {
          int reduction = (excess < lastMult - 1) ? excess : (lastMult - 1);
          uMults.SetValue(nUUniqueKnots, lastMult - reduction);
        }
      }
    }
    
    // Re-validate
    actualUSum = 0;
    for (int i = 1; i <= nUUniqueKnots; i++) {
      actualUSum += uMults.Value(i);
    }
    if (actualUSum != expectedUSum) {
      std::ostringstream oss;
      oss << "U-direction: Could not adjust multiplicities. Expected: " << expectedUSum
          << ", actual: " << actualUSum << " (numU=" << numU << ", degree=" << uBasis.degree 
          << ", nUniqueKnots=" << nUUniqueKnots << ")";
      addError(oss.str());
      return Handle(Geom_BSplineSurface)();
    }
  }
  
  for (int i = 0; i < nUUniqueKnots; i++) {
    uKnots.SetValue(i + 1, uUniqueKnots[i]);
  }

  // 3. Process V-direction knots and multiplicities
  std::vector<double> vUniqueKnots;
  i = 0;
  while (i < static_cast<int>(vBasis.knot_vector.size())) {
    double currentKnot = vBasis.knot_vector[i];
    vUniqueKnots.push_back(currentKnot);
    int j = i + 1;
    while (j < static_cast<int>(vBasis.knot_vector.size()) && 
           std::abs(vBasis.knot_vector[j] - currentKnot) < tol) {
      j++;
    }
    i = j;
  }
  
  int nVUniqueKnots = static_cast<int>(vUniqueKnots.size());
  int expectedVSum = numV + vBasis.degree + 1;
  
  TColStd_Array1OfReal vKnots(1, nVUniqueKnots);
  TColStd_Array1OfInteger vMults(1, nVUniqueKnots);
  computeKnotMultiplicities(vBasis.knot_vector, vBasis.degree, vMults);
  
  // Validate and adjust multiplicities to match expected sum
  int actualVSum = 0;
  for (int i = 1; i <= nVUniqueKnots; i++) {
    actualVSum += vMults.Value(i);
  }
  
  if (actualVSum != expectedVSum) {
    // Adjust multiplicities to match expected sum (same logic as U-direction)
    int diff = expectedVSum - actualVSum;
    
    if (diff > 0) {
      int internalKnots = nVUniqueKnots - 2;
      if (internalKnots > 0) {
        int perKnot = diff / internalKnots;
        int remainder = diff % internalKnots;
        for (int i = 2; i < nVUniqueKnots; i++) {
          int currentMult = vMults.Value(i);
          int newMult = currentMult + perKnot + (i - 2 < remainder ? 1 : 0);
          if (newMult > vBasis.degree) {
            newMult = vBasis.degree;
          }
          vMults.SetValue(i, newMult);
        }
      }
    }
    else {
      int excess = -diff;
      for (int i = 2; i < nVUniqueKnots && excess > 0; i++) {
        int currentMult = vMults.Value(i);
        int reduction = (excess < currentMult - 1) ? excess : (currentMult - 1);
        vMults.SetValue(i, currentMult - reduction);
        excess -= reduction;
      }
      if (excess > 0 && nVUniqueKnots > 0) {
        int firstMult = vMults.Value(1);
        if (firstMult > 1 && excess > 0) {
          int reduction = (excess < firstMult - 1) ? excess : (firstMult - 1);
          vMults.SetValue(1, firstMult - reduction);
          excess -= reduction;
        }
      }
      if (excess > 0 && nVUniqueKnots > 1) {
        int lastMult = vMults.Value(nVUniqueKnots);
        if (lastMult > 1 && excess > 0) {
          int reduction = (excess < lastMult - 1) ? excess : (lastMult - 1);
          vMults.SetValue(nVUniqueKnots, lastMult - reduction);
        }
      }
    }
    
    // Re-validate
    actualVSum = 0;
    for (int i = 1; i <= nVUniqueKnots; i++) {
      actualVSum += vMults.Value(i);
    }
    if (actualVSum != expectedVSum) {
      std::ostringstream oss;
      oss << "V-direction: Could not adjust multiplicities. Expected: " << expectedVSum
          << ", actual: " << actualVSum << " (numV=" << numV << ", degree=" << vBasis.degree 
          << ", nUniqueKnots=" << nVUniqueKnots << ")";
      addError(oss.str());
      return Handle(Geom_BSplineSurface)();
    }
  }
  
  for (int i = 0; i < nVUniqueKnots; i++) {
    vKnots.SetValue(i + 1, vUniqueKnots[i]);
  }

  // 4. Create surface
  Handle(Geom_BSplineSurface) surface;
  if (cp.is_rational) {
    // Rational case - would need weights, but for now treat as non-rational
    // TODO: Support rational surfaces if weights are provided in JSON
    addError("Rational surfaces not yet supported (treating as non-rational)");
    surface = new Geom_BSplineSurface(poles, uKnots, vKnots, uMults, vMults,
                                      uBasis.degree, vBasis.degree);
  }
  else {
    surface = new Geom_BSplineSurface(poles, uKnots, vKnots, uMults, vMults,
                                      uBasis.degree, vBasis.degree);
  }

  return surface;
}

//=======================================================================
// function : convertCurve2D
// purpose  : Convert 2D B-spline curve
//=======================================================================
Handle(Geom2d_BSplineCurve) EzDesignToOCCTConverter::convertCurve2D(const EzCurveData& theData)
{
  const auto& cp = theData.control_points;
  const auto& basis = theData.basis;

  // 1. Reshape 2D control points
  int numPoints = cp.number_u_points;  // Actually number of control points
  
  // Validate: B-spline curve needs at least 2 poles
  if (numPoints < 2) {
    std::ostringstream oss;
    oss << "Curve has insufficient control points: " << numPoints << " (minimum 2 required)";
    addError(oss.str());
    return Handle(Geom2d_BSplineCurve)();
  }
  
  TColgp_Array1OfPnt2d poles = reshapeControlPoints2D(cp.data, numPoints);

  // 2. Process knots and multiplicities
  // Collapse repeated knots to unique knots first
  const double tol = 1e-9;
  std::vector<double> uniqueKnots;
  int i = 0;
  while (i < static_cast<int>(basis.knot_vector.size())) {
    double currentKnot = basis.knot_vector[i];
    uniqueKnots.push_back(currentKnot);
    int j = i + 1;
    while (j < static_cast<int>(basis.knot_vector.size()) && 
           std::abs(basis.knot_vector[j] - currentKnot) < tol) {
      j++;
    }
    i = j;
  }
  
  int nUniqueKnots = static_cast<int>(uniqueKnots.size());
  int expectedSum = numPoints + basis.degree + 1;
  
  TColStd_Array1OfReal knots(1, nUniqueKnots);
  TColStd_Array1OfInteger mults(1, nUniqueKnots);
  computeKnotMultiplicities(basis.knot_vector, basis.degree, mults);
  
  // Validate and adjust multiplicities to match expected sum
  int actualSum = 0;
  for (int i = 1; i <= nUniqueKnots; i++) {
    actualSum += mults.Value(i);
  }
  
  if (actualSum != expectedSum) {
    // Adjust multiplicities (same logic as surfaces)
    int diff = expectedSum - actualSum;
    if (diff > 0) {
      int internalKnots = nUniqueKnots - 2;
      if (internalKnots > 0) {
        int perKnot = diff / internalKnots;
        int remainder = diff % internalKnots;
        for (int i = 2; i < nUniqueKnots; i++) {
          int currentMult = mults.Value(i);
          int newMult = currentMult + perKnot + (i - 2 < remainder ? 1 : 0);
          if (newMult > basis.degree) {
            newMult = basis.degree;
          }
          mults.SetValue(i, newMult);
        }
      }
    }
    else {
      int excess = -diff;
      for (int i = 2; i < nUniqueKnots && excess > 0; i++) {
        int currentMult = mults.Value(i);
        int reduction = (excess < currentMult - 1) ? excess : (currentMult - 1);
        mults.SetValue(i, currentMult - reduction);
        excess -= reduction;
      }
      if (excess > 0 && nUniqueKnots > 0) {
        int firstMult = mults.Value(1);
        if (firstMult > 1 && excess > 0) {
          int reduction = (excess < firstMult - 1) ? excess : (firstMult - 1);
          mults.SetValue(1, firstMult - reduction);
          excess -= reduction;
        }
      }
      if (excess > 0 && nUniqueKnots > 1) {
        int lastMult = mults.Value(nUniqueKnots);
        if (lastMult > 1 && excess > 0) {
          int reduction = (excess < lastMult - 1) ? excess : (lastMult - 1);
          mults.SetValue(nUniqueKnots, lastMult - reduction);
        }
      }
    }
  }

  for (int i = 0; i < nUniqueKnots; i++) {
    knots.SetValue(i + 1, uniqueKnots[i]);
  }

  // 3. Create 2D curve
  Handle(Geom2d_BSplineCurve) curve2d =
    new Geom2d_BSplineCurve(poles, knots, mults, basis.degree);

  return curve2d;
}

//=======================================================================
// function : convertCurve3D
// purpose  : Map 2D curve to 3D space via surface evaluation
//=======================================================================
Handle(Geom_BSplineCurve) EzDesignToOCCTConverter::convertCurve3D(
  const Handle(Geom2d_BSplineCurve)& theCurve2d,
  const Handle(Geom_BSplineSurface)& theSurface,
  double theUMin,
  double theUMax)
{
  // Use adaptive sampling based on curve complexity
  int numSamples = computeSampleCount(theCurve2d);
  if (numSamples < 2) {
    numSamples = 20;  // Minimum samples
  }

  // Sample 2D curve and evaluate on surface
  Handle(TColgp_HArray1OfPnt) points3d = new TColgp_HArray1OfPnt(1, numSamples);
  Handle(TColStd_HArray1OfReal) parameters = new TColStd_HArray1OfReal(1, numSamples);

  for (int i = 0; i < numSamples; i++) {
    double t = theUMin + (theUMax - theUMin) * i / (numSamples - 1.0);
    parameters->SetValue(i + 1, t);
    gp_Pnt2d p2d = theCurve2d->Value(t);
    gp_Pnt p3d = theSurface->Value(p2d.X(), p2d.Y());
    points3d->SetValue(i + 1, p3d);
  }

  // Create 3D B-spline curve from sampled points using interpolation
  // Use degree 3 for smooth curves
  GeomAPI_Interpolate interpolator(points3d, parameters, Standard_False, Precision::Confusion());
  interpolator.Perform();

  if (!interpolator.IsDone()) {
    // Fallback: create degree 1 (linear) curve
    // Ensure we have at least 2 points
    if (numSamples < 2) {
      addError("Insufficient samples for 3D curve creation: " + std::to_string(numSamples));
      return Handle(Geom_BSplineCurve)();
    }
    
    TColgp_Array1OfPnt poles3d(1, numSamples);
    for (int i = 1; i <= numSamples; i++) {
      poles3d.SetValue(i, points3d->Value(i));
    }
    TColStd_Array1OfReal knots3d(1, numSamples);
    TColStd_Array1OfInteger mults3d(1, numSamples);
    for (int i = 1; i <= numSamples; i++) {
      knots3d.SetValue(i, (i - 1.0) / (numSamples - 1.0));
      mults3d.SetValue(i, (i == 1 || i == numSamples) ? 2 : 1);
    }
    return new Geom_BSplineCurve(poles3d, knots3d, mults3d, 1);
  }

  Handle(Geom_BSplineCurve) result = interpolator.Curve();
  if (result.IsNull() || result->NbPoles() < 2) {
    addError("Interpolated 3D curve has insufficient poles: " + 
             (result.IsNull() ? std::string("null") : std::to_string(result->NbPoles())));
    return Handle(Geom_BSplineCurve)();
  }
  return result;
}

//=======================================================================
// function : computeKnotMultiplicities
// purpose  : Compute knot multiplicities from knot vector
//=======================================================================
void EzDesignToOCCTConverter::computeKnotMultiplicities(
  const std::vector<double>& theKnots,
  int theDegree,
  TColStd_Array1OfInteger& theMultiplicities)
{
  int nKnots = static_cast<int>(theKnots.size());

  if (nKnots < 2) {
    addError("Insufficient knots in knot vector");
    return;
  }

  // The knot vector may contain repeated knots.
  // We need to collapse them to unique knots and compute multiplicities.
  // For non-periodic B-splines:
  // - First and last unique knots should have multiplicity = degree + 1 (clamped)
  // - Internal unique knots have multiplicity = count of repetitions (max = degree)

  const double tol = 1e-9;
  std::vector<double> uniqueKnots;
  std::vector<int> uniqueMults;
  
  int i = 0;
  while (i < nKnots) {
    double currentKnot = theKnots[i];
    int mult = 1;
    
    // Count consecutive repetitions of this knot
    int j = i + 1;
    while (j < nKnots && std::abs(theKnots[j] - currentKnot) < tol) {
      mult++;
      j++;
    }
    
    uniqueKnots.push_back(currentKnot);
    uniqueMults.push_back(mult);
    i = j;
  }
  
  int nUniqueKnots = static_cast<int>(uniqueKnots.size());
  
  if (theMultiplicities.Length() != nUniqueKnots) {
    std::ostringstream oss;
    oss << "Knot multiplicities array size mismatch: expected " << nUniqueKnots
        << " unique knots, but array size is " << theMultiplicities.Length();
    addError(oss.str());
    return;
  }
  
  // Set multiplicities for unique knots
  // For non-periodic B-splines: numPoles = sum(multiplicities) - degree - 1
  // We need to ensure the sum matches the expected value
  // If we have too many unique knots, we may need to adjust multiplicities
  
  for (int idx = 0; idx < nUniqueKnots; idx++) {
    bool isFirst = (idx == 0);
    bool isLast = (idx == nUniqueKnots - 1);
    int mult = uniqueMults[idx];
    
    if (isFirst) {
      // First unique knot: multiplicity = degree + 1 (clamped)
      mult = theDegree + 1;
    }
    else if (isLast) {
      // Last unique knot: multiplicity = degree + 1 (clamped)
      mult = theDegree + 1;
    }
    else {
      // Internal unique knot: use counted multiplicity, but clamp to degree
      if (mult > theDegree) {
        mult = theDegree;
      }
      if (mult < 1) {
        mult = 1;
      }
    }
    
    theMultiplicities.SetValue(idx + 1, mult);
  }
  
  // Validate and adjust if needed
  // If the sum is too large, we may need to reduce internal multiplicities
  // This can happen if the JSON provides more unique knots than expected
  int sumMults = 0;
  for (int i = 1; i <= nUniqueKnots; i++) {
    sumMults += theMultiplicities.Value(i);
  }
  
  // If sum is too large and we have internal knots, try to reduce them
  // This is a heuristic - ideally the JSON should provide correct knot vector
  if (sumMults > (theDegree + 1) * 2 + (nUniqueKnots - 2) && nUniqueKnots > 2) {
    // Sum is too large - reduce internal multiplicities if possible
    int excess = sumMults - ((theDegree + 1) * 2 + (nUniqueKnots - 2));
    for (int idx = 2; idx < nUniqueKnots; idx++) {
      if (excess <= 0) break;
      int currentMult = theMultiplicities.Value(idx);
      if (currentMult > 1) {
        int reduction = (excess < currentMult - 1) ? excess : (currentMult - 1);
        theMultiplicities.SetValue(idx, currentMult - reduction);
        excess -= reduction;
      }
    }
  }
}

//=======================================================================
// function : reshapeControlPoints3D
// purpose  : Reshape 3D control points from flat array to 2D grid
//=======================================================================
TColgp_Array2OfPnt EzDesignToOCCTConverter::reshapeControlPoints3D(
  const std::vector<double>& theFlatData,
  int theNumU,
  int theNumV)
{
  TColgp_Array2OfPnt poles(1, theNumU, 1, theNumV);

  for (int i = 0; i < theNumU; i++) {
    for (int j = 0; j < theNumV; j++) {
      int idx = (i * theNumV + j) * 3;
      if (idx + 2 < static_cast<int>(theFlatData.size())) {
        gp_Pnt point(theFlatData[idx], theFlatData[idx + 1], theFlatData[idx + 2]);
        poles.SetValue(i + 1, j + 1, point);
      }
    }
  }

  return poles;
}

//=======================================================================
// function : reshapeControlPoints2D
// purpose  : Reshape 2D control points from flat array to 1D array
//=======================================================================
TColgp_Array1OfPnt2d EzDesignToOCCTConverter::reshapeControlPoints2D(
  const std::vector<double>& theFlatData,
  int theNumPoints)
{
  TColgp_Array1OfPnt2d poles(1, theNumPoints);

  for (int i = 0; i < theNumPoints; i++) {
    int idx = i * 2;  // 2D: u, v coordinates
    if (idx + 1 < static_cast<int>(theFlatData.size())) {
      gp_Pnt2d point(theFlatData[idx], theFlatData[idx + 1]);
      poles.SetValue(i + 1, point);
    }
  }

  return poles;
}

//=======================================================================
// function : computeSampleCount
// purpose  : Compute adaptive sample count based on curve complexity
//=======================================================================
int EzDesignToOCCTConverter::computeSampleCount(const Handle(Geom2d_BSplineCurve)& theCurve2d)
{
  if (theCurve2d.IsNull()) {
    return 20;  // Default
  }

  // Base sample count on degree and number of control points
  int degree = theCurve2d->Degree();
  int numPoles = theCurve2d->NbPoles();

  // Adaptive sampling: more samples for higher degree/complexity
  int baseSamples = 20;
  int additionalSamples = degree * 2 + (numPoles / 5);
  int totalSamples = baseSamples + additionalSamples;

  // Clamp between 20 and 100
  if (totalSamples < 20) {
    totalSamples = 20;
  }
  if (totalSamples > 100) {
    totalSamples = 100;
  }

  return totalSamples;
}

//=======================================================================
// function : getVertex
// purpose  : Get vertex by ID
//=======================================================================
const EzVertex& EzDesignToOCCTConverter::getVertex(int theId) const
{
  return myReader.GetVertex(theId);
}

//=======================================================================
// function : getEdge
// purpose  : Get edge by ID
//=======================================================================
const EzEdge& EzDesignToOCCTConverter::getEdge(int theId) const
{
  return myReader.GetEdge(theId);
}

//=======================================================================
// function : getHalfEdge
// purpose  : Get half-edge by ID
//=======================================================================
const EzHalfEdge& EzDesignToOCCTConverter::getHalfEdge(int theId) const
{
  return myReader.GetHalfEdge(theId);
}

//=======================================================================
// function : getLoop
// purpose  : Get loop by ID
//=======================================================================
const EzLoop& EzDesignToOCCTConverter::getLoop(int theId) const
{
  return myReader.GetLoop(theId);
}

//=======================================================================
// function : getFace
// purpose  : Get face by ID
//=======================================================================
const EzFace& EzDesignToOCCTConverter::getFace(int theId) const
{
  return myReader.GetFace(theId);
}

//=======================================================================
// function : getShell
// purpose  : Get shell by ID
//=======================================================================
const EzShell& EzDesignToOCCTConverter::getShell(int theId) const
{
  return myReader.GetShell(theId);
}

//=======================================================================
// function : getNextHalfEdge
// purpose  : Get next half-edge in chain
//=======================================================================
const EzHalfEdge& EzDesignToOCCTConverter::getNextHalfEdge(int theId) const
{
  const EzHalfEdge& he = getHalfEdge(theId);
  if (he.id == 0) {
    static EzHalfEdge empty;
    return empty;
  }
  return getHalfEdge(he.next_id);
}

//=======================================================================
// function : HasErrors
// purpose  : Check for errors
//=======================================================================
Standard_Boolean EzDesignToOCCTConverter::HasErrors() const
{
  return !myErrors.empty();
}

//=======================================================================
// function : GetErrors
// purpose  : Get error messages
//=======================================================================
const std::vector<std::string>& EzDesignToOCCTConverter::GetErrors() const
{
  return myErrors;
}

//=======================================================================
// function : addError
// purpose  : Add error message
//=======================================================================
void EzDesignToOCCTConverter::addError(const std::string& theError)
{
  myErrors.push_back(theError);
}

