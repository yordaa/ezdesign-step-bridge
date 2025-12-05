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
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cstdio>

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
// function : expandPath
// purpose  : Expand ~ to home directory
//=======================================================================
std::string expandPath(const char* path)
{
  if (path == nullptr || path[0] != '~') {
    return std::string(path);
  }
  
  const char* home = std::getenv("HOME");
  if (home == nullptr) {
    return std::string(path);
  }
  
  if (path[1] == '\0' || path[1] == '/') {
    return std::string(home) + (path[1] == '/' ? (path + 1) : "");
  }
  
  return std::string(path);
}

//=======================================================================
// function : findJson2Step
// purpose  : Find the json2step executable path
//=======================================================================
std::string findJson2Step()
{
  // Check environment variable first
  const char* envPath = std::getenv("JSON2STEP_PATH");
  if (envPath != nullptr) {
    return std::string(envPath);
  }
  
  // Try common build paths
  const char* candidates[] = {
    "build/mac64/clang/bin/json2step",
    "../../mac64/clang/bin/json2step",
    "../mac64/clang/bin/json2step",
    "json2step"  // Fallback to PATH
  };
  
  for (const char* candidate : candidates) {
    std::ifstream check(candidate);
    if (check.good()) {
      check.close();
      return std::string(candidate);
    }
  }
  
  // Default fallback
  return std::string("json2step");
}

//=======================================================================
// function : testModel
// purpose  : Test conversion and validation of a single model
//=======================================================================
int testModel(const char* jsonFile, const char* stepFile, int expectedFaces, const char* modelName, const std::string& json2stepPath)
{
  std::cout << "\n========================================" << std::endl;
  std::cout << "Testing: " << modelName << std::endl;
  std::cout << "========================================" << std::endl;
  
  // Step 1: Convert JSON to STEP using json2step
  std::cout << "Step 1: Converting " << jsonFile << " to STEP..." << std::endl;
  std::string cmd = json2stepPath + " \"" + jsonFile + "\" \"" + stepFile + "\"";
  int result = system(cmd.c_str());
  
  if (result != 0) {
    std::cerr << "ERROR: json2step conversion failed for " << modelName << std::endl;
    return 1;
  }
  
  // Check if STEP file was created
  std::ifstream fileCheck(stepFile);
  if (!fileCheck.good()) {
    std::cerr << "ERROR: STEP file was not created: " << stepFile << std::endl;
    return 1;
  }
  fileCheck.close();
  std::cout << "✓ STEP file created successfully" << std::endl;
  
  // Step 2: Read STEP file back and validate
  std::cout << "Step 2: Reading STEP file back..." << std::endl;
  
  try {
    OCC_CATCH_SIGNALS
    
    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(stepFile);
    
    if (status != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to read STEP file (status: " << status << ")" << std::endl;
      return 1;
    }
    
    std::cout << "✓ STEP file read successfully" << std::endl;
    
    // Step 3: Transfer shapes
    std::cout << "Step 3: Transferring shapes..." << std::endl;
    int numRoots = reader.NbRootsForTransfer();
    if (numRoots == 0) {
      std::cerr << "ERROR: No root entities found in STEP file" << std::endl;
      return 1;
    }
    
    int numTransferred = reader.TransferRoots();
    if (numTransferred == 0) {
      std::cerr << "ERROR: Failed to transfer any roots" << std::endl;
      return 1;
    }
    
    TopoDS_Shape shape = reader.OneShape();
    if (shape.IsNull()) {
      std::cerr << "ERROR: No shape was transferred" << std::endl;
      return 1;
    }
    
    std::cout << "✓ Transferred " << numTransferred << " root(s)" << std::endl;
    
    // Step 4: Validate face count
    std::cout << "Step 4: Validating face count..." << std::endl;
    int numFaces = countShapes(shape, TopAbs_FACE);
    int numEdges = countShapes(shape, TopAbs_EDGE);
    int numVertices = countShapes(shape, TopAbs_VERTEX);
    
    std::cout << "  Faces: " << numFaces << " (expected: " << expectedFaces << ")" << std::endl;
    std::cout << "  Edges: " << numEdges << std::endl;
    std::cout << "  Vertices: " << numVertices << std::endl;
    
    if (numFaces != expectedFaces) {
      std::cerr << "ERROR: Face count mismatch! Expected " << expectedFaces 
                << ", got " << numFaces << std::endl;
      return 1;
    }
    
    if (numFaces == 0 || numEdges == 0 || numVertices == 0) {
      std::cerr << "ERROR: Shape appears incomplete" << std::endl;
      return 1;
    }
    
    std::cout << "✓ Face count matches expected value" << std::endl;
    std::cout << "✓ " << modelName << " test PASSED" << std::endl;
    return 0;
  }
  catch (Standard_Failure const& ex) {
    std::cerr << "ERROR: Exception: " << ex.GetMessageString() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "ERROR: Unknown exception" << std::endl;
    return 1;
  }
}

//=======================================================================
// function : main
// purpose  : Run tests for single-face, double-face, and 3-faces models
//=======================================================================
int main(int argc, char* argv[])
{
  // Default paths - can be overridden via command line
  std::string singleFaceJson = (argc > 1) ? argv[1] : expandPath("~/Downloads/20251203-single-face.ezd");
  std::string doubleFaceJson = (argc > 2) ? argv[2] : expandPath("~/Downloads/20251203-double-face.ezd");
  std::string threeFacesJson = (argc > 3) ? argv[3] : expandPath("~/Downloads/20251204-3-faces.ezd");
  
  std::cout << "========================================" << std::endl;
  std::cout << "json2step Basic Models Test Suite" << std::endl;
  std::cout << "========================================" << std::endl;
  
  // Find json2step executable
  std::string json2stepPath = findJson2Step();
  std::cout << "Using json2step: " << json2stepPath << std::endl;
  
  int totalFailures = 0;
  
  // Test 1: Single-face
  totalFailures += testModel(
    singleFaceJson.c_str(),
    "/tmp/test-single-face.step",
    1,
    "Single-face",
    json2stepPath
  );
  
  // Test 2: Double-face
  totalFailures += testModel(
    doubleFaceJson.c_str(),
    "/tmp/test-double-face.step",
    2,
    "Double-face",
    json2stepPath
  );
  
  // Test 3: 3-faces
  totalFailures += testModel(
    threeFacesJson.c_str(),
    "/tmp/test-3-faces.step",
    3,
    "3-faces",
    json2stepPath
  );
  
  // Summary
  std::cout << "\n========================================" << std::endl;
  std::cout << "Test Summary" << std::endl;
  std::cout << "========================================" << std::endl;
  
  if (totalFailures == 0) {
    std::cout << "✓ All tests PASSED" << std::endl;
    return 0;
  } else {
    std::cerr << "✗ " << totalFailures << " test(s) FAILED" << std::endl;
    return 1;
  }
}

