// Created on: 2026
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#include <Standard.hxx>
#include <Standard_ErrorHandler.hxx>
#include <Standard_Failure.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Shape.hxx>

#include "ezd_to_step.h"

#include <filesystem>
#include <iostream>
#include <string>

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

std::filesystem::path executableDirectory(const char* theExecutablePath)
{
  const std::filesystem::path aPath(theExecutablePath);
  if (aPath.has_parent_path()) {
    return aPath.parent_path();
  }

  return std::filesystem::current_path();
}
}

int main(int argc, char* argv[])
{
  const std::filesystem::path anInputPath = argc > 1
    ? std::filesystem::path(argv[1])
    : executableDirectory(argv[0]) / "20260427.ezd";
  const std::filesystem::path anOutputPath =
    std::filesystem::temp_directory_path() / "ezd2step-20260427-conversion.step";

  if (!std::filesystem::exists(anInputPath)) {
    std::cerr << "ERROR: EZD fixture does not exist: " << anInputPath << std::endl;
    return 1;
  }

  std::filesystem::remove(anOutputPath);

  ezd_to_step_options anOptions = {0, nullptr};
  std::cout << "Converting " << anInputPath << std::endl;
  const int aConversionResult = ezd_to_step(
    anInputPath.string().c_str(),
    anOutputPath.string().c_str(),
    &anOptions);
  if (aConversionResult != 0) {
    std::cerr << "ERROR: ezd2step conversion failed with status " << aConversionResult << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(anOutputPath)) {
    std::cerr << "ERROR: STEP output was not created: " << anOutputPath << std::endl;
    return 1;
  }

  try {
    OCC_CATCH_SIGNALS

    STEPControl_Reader aReader;
    const IFSelect_ReturnStatus aStatus = aReader.ReadFile(anOutputPath.string().c_str());
    if (aStatus != IFSelect_RetDone) {
      std::cerr << "ERROR: generated STEP file is not readable; status " << aStatus << std::endl;
      return 1;
    }

    if (aReader.NbRootsForTransfer() == 0) {
      std::cerr << "ERROR: generated STEP file has no transferable roots" << std::endl;
      return 1;
    }

    if (aReader.TransferRoots() == 0) {
      std::cerr << "ERROR: generated STEP roots did not transfer to OCCT shapes" << std::endl;
      return 1;
    }

    const TopoDS_Shape aShape = aReader.OneShape();
    if (aShape.IsNull()) {
      std::cerr << "ERROR: transferred STEP shape is null" << std::endl;
      return 1;
    }

    const int aFaceCount = countShapes(aShape, TopAbs_FACE);
    const int anEdgeCount = countShapes(aShape, TopAbs_EDGE);
    const int aVertexCount = countShapes(aShape, TopAbs_VERTEX);
    std::cout << "Converted shape statistics:" << std::endl;
    std::cout << "  Faces: " << aFaceCount << std::endl;
    std::cout << "  Edges: " << anEdgeCount << std::endl;
    std::cout << "  Vertices: " << aVertexCount << std::endl;

    if (aFaceCount == 0 || anEdgeCount == 0 || aVertexCount == 0) {
      std::cerr << "ERROR: converted STEP shape is incomplete" << std::endl;
      return 1;
    }
  }
  catch (const Standard_Failure& theFailure) {
    std::cerr << "ERROR: OCCT exception: " << theFailure.GetMessageString() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "ERROR: unknown exception while validating generated STEP" << std::endl;
    return 1;
  }

  std::filesystem::remove(anOutputPath);
  std::cout << "20260427.ezd conversion test passed" << std::endl;
  return 0;
}
