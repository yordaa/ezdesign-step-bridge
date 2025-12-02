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
#include <Message.hxx>
#include <STEPControl_Reader.hxx>
#include <XSControl_WorkSession.hxx>
#include <IFSelect_WorkLibrary.hxx>
#include <Interface_Protocol.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <IFSelect_PrintCount.hxx>
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
#include <Interface_CheckIterator.hxx>
#include <Interface_Check.hxx>
#include <StepData_StepModel.hxx>
#include <StepFile_Read.hxx>
#include <StepData_Protocol.hxx>
#include <Message_Printer.hxx>
#include <Message_PrinterOStream.hxx>
#include <IFSelect_PrintCount.hxx>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <csignal>
#include <cstdlib>

// Signal handler for debugging
void signalHandler(int sig) {
  std::cerr << "\nERROR: Signal " << sig << " caught (likely segfault)" << std::endl;
  std::cerr << "This indicates a crash during STEP file reading." << std::endl;
  exit(1);
}

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
  // Install signal handler for debugging
  signal(SIGSEGV, signalHandler);
  signal(SIGABRT, signalHandler);
  
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
    // STEPControl_Reader constructor automatically initializes STEPControl_Controller
    std::cout << "Creating STEPControl_Reader..." << std::endl;
    STEPControl_Reader reader;
    std::cout << "STEPControl_Reader created successfully" << std::endl;
    
    // Check if WorkSession is valid
    if (reader.WS().IsNull()) {
      std::cerr << "ERROR: WorkSession is null" << std::endl;
      return 1;
    }
    std::cout << "WorkSession is valid" << std::endl;
    
    // Check WorkLibrary and Protocol before reading
    if (reader.WS()->WorkLibrary().IsNull()) {
      std::cerr << "ERROR: WorkLibrary is null" << std::endl;
      return 1;
    }
    if (reader.WS()->Protocol().IsNull()) {
      std::cerr << "ERROR: Protocol is null" << std::endl;
      return 1;
    }
    std::cout << "WorkLibrary and Protocol are valid" << std::endl;
    
    std::cout << "Reading STEP file: " << stepFile << "..." << std::endl;
    std::cout.flush();
    
    IFSelect_ReturnStatus status = IFSelect_RetVoid;
    try {
      OCC_CATCH_SIGNALS
      status = reader.ReadFile(stepFile);
    }
    catch (Standard_Failure const& ex) {
      std::cerr << "ERROR: Exception during ReadFile: " << ex.GetMessageString() << std::endl;
      return 1;
    }
    catch (...) {
      std::cerr << "ERROR: Unknown exception during ReadFile" << std::endl;
      return 1;
    }
    
    std::cout << "ReadFile completed with status: " << status << std::endl;
    std::cout << "  IFSelect_RetDone = 0 (success)" << std::endl;
    std::cout << "  IFSelect_RetVoid = 1 (void/not found)" << std::endl;
    std::cout << "  IFSelect_RetError = 2 (error)" << std::endl;
    std::cout << "  IFSelect_RetFail = 3 (fail)" << std::endl;
    
    // Try to get model and check messages even if read failed
    Handle(StepData_StepModel) model = reader.StepModel();
    
    if (!model.IsNull()) {
      std::cout << "\n=== MODEL CHECK MESSAGES ===" << std::endl;
      Interface_CheckIterator checks = model->Check();
      if (!checks.IsEmpty(Standard_False)) {
        std::cerr << "Model has check messages (failures and/or warnings)" << std::endl;
        checks.Print(std::cerr, Standard_False, 0);
      } else {
        std::cout << "No check messages from model." << std::endl;
      }
    } else {
      std::cerr << "Model is null - file may not have been parsed at all." << std::endl;
    }
    
    // Print check messages from reader
    std::cout << "\n=== READER CHECK MESSAGES ===" << std::endl;
    reader.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
    
    if (status != IFSelect_RetDone) {
      std::cerr << "\n=== STEP FILE READ FAILED ===" << std::endl;
      std::cerr << "Status: " << status << std::endl;
      std::cerr << "The generated STEP file is invalid and cannot be read by OCCT." << std::endl;
      return 1;
    }

    std::cout << "\nSTEP file read successfully." << std::endl;

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

