// Created on: 2026
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#include <BRepAdaptor_Surface.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <Geom2d_Curve.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Precision.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <STEPControl_Reader.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopAbs_State.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <utility>

namespace
{
int countShapes(const TopoDS_Shape& theShape, TopAbs_ShapeEnum theType)
{
  int aCount = 0;
  for (TopExp_Explorer anExplorer(theShape, theType); anExplorer.More(); anExplorer.Next()) {
    ++aCount;
  }
  return aCount;
}

bool checkFace(const TopoDS_Face& theFace)
{
  if (!BRepCheck_Analyzer(theFace).IsValid()) {
    std::cerr << "ERROR: imported face is invalid" << std::endl;
    return false;
  }
  if (countShapes(theFace, TopAbs_WIRE) != 1 || countShapes(theFace, TopAbs_EDGE) != 4) {
    std::cerr << "ERROR: imported face does not have one wire and four edges" << std::endl;
    return false;
  }

  Standard_Real aUMin, aUMax, aVMin, aVMax;
  BRepTools::UVBounds(theFace, aUMin, aUMax, aVMin, aVMax);
  const Standard_Real aU = (aUMin + aUMax) / 2.0;
  const Standard_Real aV = (aVMin + aVMax) / 2.0;

  BRepClass_FaceClassifier anInside(theFace, gp_Pnt2d(aU, aV), Precision::PConfusion());
  BRepClass_FaceClassifier anOutside(
    theFace,
    gp_Pnt2d(aUMax + (aUMax - aUMin), aVMax + (aVMax - aVMin)),
    Precision::PConfusion());
  if (anInside.State() != TopAbs_IN || anOutside.State() != TopAbs_OUT) {
    std::cerr << "ERROR: imported face does not retain the finite UV region" << std::endl;
    return false;
  }

  BRepAdaptor_Surface aSurface(theFace, Standard_True);
  gp_Pnt aPoint;
  gp_Vec aDU, aDV;
  aSurface.D1(aU, aV, aPoint, aDU, aDV);
  gp_Vec aNormal = aDU.Crossed(aDV);
  if (aNormal.SquareMagnitude() <= Precision::SquareConfusion()) {
    std::cerr << "ERROR: imported face has an undefined normal" << std::endl;
    return false;
  }
  if (theFace.Orientation() == TopAbs_REVERSED) {
    aNormal.Reverse();
  }
  aNormal.Normalize();

  const Standard_Real anExpectedZ = aPoint.Z() < 1.0 ? -1.0 : 1.0;
  if (aNormal.Z() * anExpectedZ <= 0.999) {
    std::cerr << "ERROR: face at Z=" << aPoint.Z() << " has oriented normal Z="
              << aNormal.Z() << ", expected " << anExpectedZ << std::endl;
    return false;
  }

  Standard_Real aTwiceSignedArea = 0.0;
  const TopoDS_Wire anOuterWire = BRepTools::OuterWire(theFace);
  for (BRepTools_WireExplorer anExplorer(anOuterWire, theFace);
       anExplorer.More();
       anExplorer.Next()) {
    const TopoDS_Edge anEdge = anExplorer.Current();
    Standard_Real aFirst, aLast;
    const Handle(Geom2d_Curve) aCurve = BRep_Tool::CurveOnSurface(anEdge, theFace, aFirst, aLast);
    if (aCurve.IsNull()) {
      std::cerr << "ERROR: imported edge has no pcurve" << std::endl;
      return false;
    }

    gp_Pnt2d aStart = aCurve->Value(aFirst);
    gp_Pnt2d anEnd = aCurve->Value(aLast);
    if (anEdge.Orientation() == TopAbs_REVERSED) {
      std::swap(aStart, anEnd);
    }
    aTwiceSignedArea += aStart.X() * anEnd.Y() - anEnd.X() * aStart.Y();
  }
  if (aTwiceSignedArea * anExpectedZ <= Precision::PConfusion()) {
    std::cerr << "ERROR: face at Z=" << aPoint.Z() << " has contextual UV area "
              << aTwiceSignedArea / 2.0 << " with the wrong winding" << std::endl;
    return false;
  }
  return true;
}
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <generated.step>" << std::endl;
    return 1;
  }

  const std::filesystem::path anOutputPath(argv[1]);
  try {
    OCC_CATCH_SIGNALS

    STEPControl_Reader aReader;
    if (aReader.ReadFile(anOutputPath.string().c_str()) != IFSelect_RetDone
        || aReader.TransferRoots() == 0) {
      std::cerr << "ERROR: orientation STEP output could not be imported" << std::endl;
      return 1;
    }

    const TopoDS_Shape aShape = aReader.OneShape();
    if (countShapes(aShape, TopAbs_FACE) != 2) {
      std::cerr << "ERROR: expected two imported faces" << std::endl;
      return 1;
    }

    for (TopExp_Explorer anExplorer(aShape, TopAbs_FACE);
         anExplorer.More();
         anExplorer.Next()) {
      if (!checkFace(TopoDS::Face(anExplorer.Current()))) {
        return 1;
      }
    }
  }
  catch (const Standard_Failure& theFailure) {
    std::cerr << "ERROR: OCCT exception: " << theFailure.GetMessageString() << std::endl;
    return 1;
  }

  std::filesystem::remove(anOutputPath);
  std::cout << "Face orientation STEP round-trip validation passed" << std::endl;
  return 0;
}
