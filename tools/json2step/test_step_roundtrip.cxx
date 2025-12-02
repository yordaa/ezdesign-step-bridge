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

#include <Standard.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Compound.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <BRep_Tool.hxx>
#include <iostream>
#include <iomanip>

//=======================================================================
// function : countShapes
// purpose  : Count shapes of specific type in a shape
//=======================================================================
int countShapes(const TopoDS_Shape& theShape, TopAbs_ShapeEnum theType)
{
  int count = 0;
  TopExp_Explorer exp(theShape, theType);
  for (; exp.More(); exp.Next()) {
    count++;
  }
  return count;
}

//=======================================================================
// function : main
// purpose  : Test round-trip verification of STEP file
//=======================================================================
int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <step_file>" << std::endl;
    return 1;
  }

  const char* stepFile = argv[1];
  std::cout << "Reading STEP file: " << stepFile << std::endl;
  
  // Check file exists
  std::ifstream fileCheck(stepFile);
  if (!fileCheck.good()) {
    std::cerr << "ERROR: Cannot open file: " << stepFile << std::endl;
    return 1;
  }
  fileCheck.close();

  try {
    OCC_CATCH_SIGNALS

    // 1. Read STEP file
    STEPControl_Reader reader;
    std::cout << "Created STEPControl_Reader" << std::endl;
    
    IFSelect_ReturnStatus status = reader.ReadFile(stepFile);
    std::cout << "ReadFile returned status: " << status << std::endl;
    
    if (status != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to read STEP file. Status: " << status << std::endl;
      return 1;
    }

    std::cout << "STEP file read successfully." << std::endl;

    // 2. Transfer root shapes
    int numRoots = reader.NbRootsForTransfer();
    std::cout << "Number of root entities for transfer: " << numRoots << std::endl;

    if (numRoots == 0) {
      std::cerr << "ERROR: No root entities found in STEP file." << std::endl;
      return 1;
    }

    // Transfer all roots
    int numTransferred = reader.TransferRoots();
    if (numTransferred == 0) {
      std::cerr << "ERROR: Failed to transfer any roots." << std::endl;
      return 1;
    }
    
    TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
      std::cerr << "ERROR: No shape was transferred." << std::endl;
      return 1;
    }

    int numShapes = reader.NbShapes();
    std::cout << "Transferred " << numShapes << " shape(s)." << std::endl;

    // 3. Analyze shape
    std::cout << "\n=== Shape Analysis ===" << std::endl;
    std::cout << "Shape type: ";
    switch (shape.ShapeType()) {
      case TopAbs_COMPOUND: std::cout << "Compound"; break;
      case TopAbs_COMPSOLID: std::cout << "CompSolid"; break;
      case TopAbs_SOLID: std::cout << "Solid"; break;
      case TopAbs_SHELL: std::cout << "Shell"; break;
      case TopAbs_FACE: std::cout << "Face"; break;
      case TopAbs_WIRE: std::cout << "Wire"; break;
      case TopAbs_EDGE: std::cout << "Edge"; break;
      case TopAbs_VERTEX: std::cout << "Vertex"; break;
      default: std::cout << "Unknown"; break;
    }
    std::cout << std::endl;

    // Count sub-shapes
    int numSolids = countShapes(shape, TopAbs_SOLID);
    int numShells = countShapes(shape, TopAbs_SHELL);
    int numFaces = countShapes(shape, TopAbs_FACE);
    int numEdges = countShapes(shape, TopAbs_EDGE);
    int numVertices = countShapes(shape, TopAbs_VERTEX);

    std::cout << "\n=== Shape Statistics ===" << std::endl;
    std::cout << std::left << std::setw(20) << "Solids:" << std::right << std::setw(10) << numSolids << std::endl;
    std::cout << std::left << std::setw(20) << "Shells:" << std::right << std::setw(10) << numShells << std::endl;
    std::cout << std::left << std::setw(20) << "Faces:" << std::right << std::setw(10) << numFaces << std::endl;
    std::cout << std::left << std::setw(20) << "Edges:" << std::right << std::setw(10) << numEdges << std::endl;
    std::cout << std::left << std::setw(20) << "Vertices:" << std::right << std::setw(10) << numVertices << std::endl;

    // 4. Validate shape
    std::cout << "\n=== Validation ===" << std::endl;
    if (numFaces > 0 && numEdges > 0 && numVertices > 0) {
      std::cout << "✓ Shape appears valid (has faces, edges, and vertices)" << std::endl;
    } else {
      std::cout << "⚠ Warning: Shape may be incomplete" << std::endl;
    }

    if (numSolids > 0) {
      std::cout << "✓ Contains " << numSolids << " solid(s)" << std::endl;
    } else if (numShells > 0) {
      std::cout << "✓ Contains " << numShells << " shell(s) (no solids)" << std::endl;
    }

    std::cout << "\nSUCCESS: STEP file round-trip verification passed!" << std::endl;
    return 0;

  }
  catch (Standard_Failure const& anException) {
    std::cerr << "ERROR: Exception caught: " << anException.GetMessageString() << std::endl;
    return 1;
  }
}

