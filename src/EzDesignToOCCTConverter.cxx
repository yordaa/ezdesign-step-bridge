// Created on: 2025
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

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
#include <TopExp.hxx>
#include <Precision.hxx>
#include <TopLoc_Location.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <BSplCLib.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Geom2d_Line.hxx>
#include <gp_Vec2d.hxx>
#include <gp_Dir2d.hxx>
#include <sstream>

EzDesignToOCCTConverter::EzDesignToOCCTConverter(const EzDesignJsonReader& theReader)
: myReader(theReader)
{
}

EzDesignToOCCTConverter::~EzDesignToOCCTConverter()
{
}

TopoDS_Shape EzDesignToOCCTConverter::ConvertBody(const EzBody& theBody)
{
  myErrors.clear();
  myVertexMap.clear();
  myEdgeMap.clear();

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

TopoDS_Shape EzDesignToOCCTConverter::convertBody(const EzBody& theBody)
{
  if (theBody.shell_ids.empty()) {
    addError("Body has no shells");
    return TopoDS_Shape();
  }

  if (theBody.shell_ids.size() == 1) {
    // Single shell - try to make solid
    const EzShell& shell = myReader.GetShell(theBody.shell_ids[0]);
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
      const EzShell& shell = myReader.GetShell(shellId);
      TopoDS_Shell occtShell = convertShell(shell);
      if (!occtShell.IsNull()) {
        builder.Add(compound, occtShell);
      }
    }

    return compound;
  }
}

TopoDS_Shell EzDesignToOCCTConverter::convertShell(const EzShell& theShell)
{
  BRep_Builder builder;
  TopoDS_Shell resultShell;
  builder.MakeShell(resultShell);

  // Convert and add all faces
  for (int faceId : theShell.face_ids) {
    const EzFace& face = myReader.GetFace(faceId);
    TopoDS_Face occtFace = convertFace(face);

    if (!occtFace.IsNull()) {
      builder.Add(resultShell, occtFace);
    }
  }

  return resultShell;
}

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
    const EzLoop& loop = myReader.GetLoop(loopId);
    TopoDS_Wire wire = convertLoop(loop, surface);

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
  if (!theFace.is_surface_normal_same) {
    resultFace.Reverse();
  }
  return resultFace;
}

TopoDS_Wire EzDesignToOCCTConverter::convertLoop(
  const EzLoop& theLoop,
  const Handle(Geom_BSplineSurface)& theSurface)
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

    const EzHalfEdge& halfEdge = myReader.GetHalfEdge(currentHeId);
    if (halfEdge.id == 0) {
      addError("Loop " + std::to_string(theLoop.id) + ": Half-edge " + std::to_string(currentHeId) + " not found");
      return TopoDS_Wire();
    }

    TopoDS_Edge edge = convertHalfEdge(halfEdge, theSurface);
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

TopoDS_Edge EzDesignToOCCTConverter::convertHalfEdge(
  const EzHalfEdge& theHalfEdge,
  const Handle(Geom_BSplineSurface)& theSurface)
{
  // 0. Skip conceptual half-edges (loop_id == 0 means conceptual only, no real meaning)
  if (theHalfEdge.loop_id == 0) {
    // This is a conceptual half-edge, should not create real topology
    return TopoDS_Edge();
  }

  // 0.5. Check if edge already exists in map (for shared edges)
  int edgeId = theHalfEdge.edge_id;
  auto edgeIt = myEdgeMap.find(edgeId);
  if (edgeIt != myEdgeMap.end()) {
    // Edge already exists - check if pcurve for this surface exists
    TopoDS_Edge existingEdge = edgeIt->second;
    
    // Check if pcurve already exists for this surface
    Standard_Real f, l;
    Handle(Geom2d_Curve) existingPCurve = BRep_Tool::CurveOnSurface(existingEdge, theSurface, TopLoc_Location(), f, l);
    
    if (existingPCurve.IsNull()) {
      // Pcurve missing for this surface - add it
      addPCurveToEdge(existingEdge, theHalfEdge, theSurface);
    }
    
    return TopoDS::Edge(existingEdge.Reversed());
  }

  // 1. Get start and end vertices
  // vertex_id is the vertex this half-edge points to (end vertex)
  const EzVertex& endVertex = myReader.GetVertex(theHalfEdge.vertex_id);
  if (endVertex.id == 0) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": End vertex " + std::to_string(theHalfEdge.vertex_id) + " not found");
    return TopoDS_Edge();
  }

  // Start vertex is the vertex the previous half-edge points to
  const EzHalfEdge& previousHalfEdge = myReader.GetHalfEdge(theHalfEdge.previous_id);
  if (previousHalfEdge.id == 0) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Previous half-edge not found");
    return TopoDS_Edge();
  }

  const EzVertex& startVertex = myReader.GetVertex(previousHalfEdge.vertex_id);
  if (startVertex.id == 0) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Start vertex " + std::to_string(previousHalfEdge.vertex_id) + " not found");
    return TopoDS_Edge();
  }

  TopoDS_Vertex vStart = convertVertex(startVertex);
  TopoDS_Vertex vEnd = convertVertex(endVertex);

  // 2. Get curve_data - try current half-edge first, then opposite if available
  EzCurveData curveData = theHalfEdge.curve_data;

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
    TopoDS_Edge newEdge = edgeMaker.Edge();
    myEdgeMap[edgeId] = newEdge;  // Store in map for sharing
    return newEdge;
  }

  // Note: ezdesign convention ensures half-edges are CCW w.r.t. face outward normal
  // The 2D curve orientation is already correct per this convention

  // 5. Get parameter range
  double uMin = curveData.basis.bounds.minimum;
  double uMax = curveData.basis.bounds.maximum;

  // 6. Create edge directly from pcurve and surface with vertices (no 3D curve needed)
  // OCCT's STEP writer will compute the 3D curve automatically if needed

  // Update vertex tolerances based on actual geometric distances
  // Evaluate pcurve endpoints in 3D space and compare with vertex positions
  gp_Pnt2d uvStart = curve2d->Value(uMin);
  gp_Pnt2d uvEnd = curve2d->Value(uMax);
  gp_Pnt curveStart3D = theSurface->Value(uvStart.X(), uvStart.Y());
  gp_Pnt curveEnd3D = theSurface->Value(uvEnd.X(), uvEnd.Y());
  
  gp_Pnt vertexStart3D = BRep_Tool::Pnt(vStart);
  gp_Pnt vertexEnd3D = BRep_Tool::Pnt(vEnd);
  
  Standard_Real distStart = curveStart3D.Distance(vertexStart3D);
  Standard_Real distEnd = curveEnd3D.Distance(vertexEnd3D);
  
  // Set tolerance to accommodate the geometric discrepancy, with a safety margin
  Standard_Real baseTol = Precision::Confusion();
  Standard_Real tolStart = BRep_Tool::Tolerance(vStart);
  Standard_Real tolEnd = BRep_Tool::Tolerance(vEnd);
  
  // Tolerance should be at least the distance, with a small safety margin (1.1x)
  Standard_Real requiredTolStart = (baseTol > distStart * 1.1) ? baseTol : (distStart * 1.1);
  Standard_Real requiredTolEnd = (baseTol > distEnd * 1.1) ? baseTol : (distEnd * 1.1);
  
  // Update only if current tolerance is insufficient
  if (tolStart < requiredTolStart || tolEnd < requiredTolEnd) {
    BRep_Builder builder;
    if (tolStart < requiredTolStart) {
      builder.UpdateVertex(vStart, requiredTolStart);
    }
    if (tolEnd < requiredTolEnd) {
      builder.UpdateVertex(vEnd, requiredTolEnd);
    }
  }

  BRepBuilderAPI_MakeEdge edgeMaker(curve2d, theSurface, vStart, vEnd, uMin, uMax);
  if (!edgeMaker.IsDone()) {
    addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to create edge from pcurve");
    return TopoDS_Edge();
  }
  TopoDS_Edge newEdge = edgeMaker.Edge();
  myEdgeMap[edgeId] = newEdge;  // Store in map for sharing
  return newEdge;
}

void EzDesignToOCCTConverter::addPCurveToEdge(
  TopoDS_Edge& theEdge,
  const EzHalfEdge& theHalfEdge,
  const Handle(Geom_BSplineSurface)& theSurface)
{
  // 1. Get curve_data - try current half-edge first, then opposite if available
  EzCurveData curveData = theHalfEdge.curve_data;
  bool hasCurveData = !curveData.control_points.data.empty() &&
                      curveData.control_points.number_v_points >= 2;

  // If no curve_data, try opposite half-edge
  if (!hasCurveData && theHalfEdge.opposite_id != 0) {
    const EzHalfEdge& oppositeHe = myReader.GetHalfEdge(theHalfEdge.opposite_id);
    if (oppositeHe.id != 0 && !oppositeHe.curve_data.control_points.data.empty() &&
        oppositeHe.curve_data.control_points.number_v_points >= 2) {
      curveData = oppositeHe.curve_data;
      hasCurveData = true;
    }
  }

  Handle(Geom2d_Curve) pcurve;
  Standard_Real uMin, uMax;

  if (hasCurveData) {
    // Convert 2D curve from curve_data
    Handle(Geom2d_BSplineCurve) curve2d = convertCurve2D(curveData);
    if (!curve2d.IsNull()) {
      pcurve = curve2d;
      uMin = curveData.basis.bounds.minimum;
      uMax = curveData.basis.bounds.maximum;
    }
  }

  // If still no pcurve, generate a straight 2D line by projecting edge endpoints
  if (pcurve.IsNull()) {
    // Get edge vertices
    TopoDS_Vertex vStart, vEnd;
    TopExp::Vertices(theEdge, vStart, vEnd);
    gp_Pnt p1 = BRep_Tool::Pnt(vStart);
    gp_Pnt p2 = BRep_Tool::Pnt(vEnd);

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

      pcurve = line2d;
      uMin = 0.0;
      uMax = dir.Magnitude();
    } else {
      addError("HalfEdge " + std::to_string(theHalfEdge.id) + ": Failed to project edge endpoints for pcurve");
      return;
    }
  }

  // Add pcurve to edge using BRep_Builder
  BRep_Builder builder;
  builder.UpdateEdge(theEdge, pcurve, theSurface, TopLoc_Location(), Precision::Confusion());
  builder.Range(theEdge, theSurface, TopLoc_Location(), uMin, uMax);
}

TopoDS_Vertex EzDesignToOCCTConverter::convertVertex(const EzVertex& theVertex)
{
  // Check if this vertex has already been converted
  auto it = myVertexMap.find(theVertex.id);
  if (it != myVertexMap.end()) {
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
  myVertexMap[theVertex.id] = vertex;
  return vertex;
}

Handle(Geom_BSplineSurface) EzDesignToOCCTConverter::convertSurface(const EzSurfaceData& theData)
{
  const auto& cp = theData.control_points;
  const auto& uBasis = theData.u_basis;
  const auto& vBasis = theData.v_basis;
  std::vector<double> uKnotData, vKnotData;
  std::vector<int> uMultData, vMultData;
  if (!buildKnotData(uBasis.knot_vector, uBasis.degree, cp.number_u_points, uKnotData, uMultData)
   || !buildKnotData(vBasis.knot_vector, vBasis.degree, cp.number_v_points, vKnotData, vMultData)) {
    return Handle(Geom_BSplineSurface)();
  }

  TColgp_Array2OfPnt poles = reshapeControlPoints3D(cp.data, cp.number_u_points, cp.number_v_points);
  TColStd_Array1OfReal uKnots(1, uKnotData.size()), vKnots(1, vKnotData.size());
  TColStd_Array1OfInteger uMults(1, uMultData.size()), vMults(1, vMultData.size());
  for (int i = 0; i < static_cast<int>(uKnotData.size()); ++i) { uKnots.SetValue(i + 1, uKnotData[i]); uMults.SetValue(i + 1, uMultData[i]); }
  for (int i = 0; i < static_cast<int>(vKnotData.size()); ++i) { vKnots.SetValue(i + 1, vKnotData[i]); vMults.SetValue(i + 1, vMultData[i]); }
  return new Geom_BSplineSurface(poles, uKnots, vKnots, uMults, vMults, uBasis.degree, vBasis.degree);
}

Handle(Geom2d_BSplineCurve) EzDesignToOCCTConverter::convertCurve2D(const EzCurveData& theData)
{
  const auto& cp = theData.control_points;
  const auto& basis = theData.basis;

  // 1. Reshape 2D control points
  int numPoints = cp.number_v_points;  // For curves: number_v_points is the number of control points
  
  // Validate: B-spline curve needs at least 2 poles
  if (numPoints < 2) {
    std::ostringstream oss;
    oss << "Curve has insufficient control points: " << numPoints << " (minimum 2 required)";
    addError(oss.str());
    return Handle(Geom2d_BSplineCurve)();
  }
  
  std::vector<double> knotData;
  std::vector<int> multData;
  if (!buildKnotData(basis.knot_vector, basis.degree, numPoints, knotData, multData)) {
    return Handle(Geom2d_BSplineCurve)();
  }

  TColgp_Array1OfPnt2d poles = reshapeControlPoints2D(cp.data, numPoints);
  TColStd_Array1OfReal knots(1, knotData.size());
  TColStd_Array1OfInteger mults(1, multData.size());
  for (int i = 0; i < static_cast<int>(knotData.size()); ++i) {
    knots.SetValue(i + 1, knotData[i]);
    mults.SetValue(i + 1, multData[i]);
  }
  return new Geom2d_BSplineCurve(poles, knots, mults, basis.degree);
}

bool EzDesignToOCCTConverter::buildKnotData(
  const std::vector<double>& theKnotSequence,
  int theDegree,
  int thePoleCount,
  std::vector<double>& theKnots,
  std::vector<int>& theMultiplicities)
{
  if (theKnotSequence.size() < 2) {
    addError("Insufficient knots in knot vector");
    return false;
  }
  TColStd_Array1OfReal sequence(1, theKnotSequence.size());
  for (int i = 0; i < static_cast<int>(theKnotSequence.size()); ++i) {
    sequence.SetValue(i + 1, theKnotSequence[i]);
  }
  const int knotCount = BSplCLib::KnotsLength(sequence);
  TColStd_Array1OfReal knots(1, knotCount);
  TColStd_Array1OfInteger mults(1, knotCount);
  BSplCLib::Knots(sequence, knots, mults);
  if (BSplCLib::NbPoles(theDegree, Standard_False, mults) != thePoleCount) {
    addError("Knot multiplicities do not match control-point count");
    return false;
  }
  theKnots.clear();
  theMultiplicities.clear();
  for (int i = 1; i <= knotCount; ++i) {
    theKnots.push_back(knots.Value(i));
    theMultiplicities.push_back(mults.Value(i));
  }
  return true;
}

TColgp_Array2OfPnt EzDesignToOCCTConverter::reshapeControlPoints3D(
  const std::vector<double>& theFlatData,
  int theNumU,
  int theNumV)
{
  TColgp_Array2OfPnt poles(1, theNumU, 1, theNumV);

  for (int i = 0; i < theNumU; i++) {
    for (int j = 0; j < theNumV; j++) {
      int idx = (j * theNumU + i) * 3;
      if (idx + 2 < static_cast<int>(theFlatData.size())) {
        gp_Pnt point(theFlatData[idx], theFlatData[idx + 1], theFlatData[idx + 2]);
        poles.SetValue(i + 1, j + 1, point);
      }
    }
  }

  return poles;
}

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

const std::vector<std::string>& EzDesignToOCCTConverter::GetErrors() const
{
  return myErrors;
}

void EzDesignToOCCTConverter::addError(const std::string& theError)
{
  myErrors.push_back(theError);
}
