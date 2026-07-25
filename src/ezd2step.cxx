// Created on: 2025
// Created by: EzDesign to STEP Converter
// Copyright (c) 2025 Yang Song. All rights reserved.
//
// This file is proprietary and confidential. Unauthorized copying, modification,
// distribution, or use of this file, via any medium, is strictly prohibited.
// See LICENSE file in this directory for terms and conditions.

#include "EzDesignJsonReader.hxx"
#include "EzDesignToOCCTConverter.hxx"

#include <Standard.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <IFSelect_PrintCount.hxx>
#include <TCollection_AsciiString.hxx>
#include <StepData_StepModel.hxx>
#include <Interface_Check.hxx>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

// Version definitions (from CMake)
#ifndef EZD2STEP_VERSION_MAJOR
#define EZD2STEP_VERSION_MAJOR 1
#endif
#ifndef EZD2STEP_VERSION_MINOR
#define EZD2STEP_VERSION_MINOR 0
#endif
#ifndef EZD2STEP_VERSION_PATCH
#define EZD2STEP_VERSION_PATCH 0
#endif
#ifndef EZD2STEP_VERSION
#define EZD2STEP_VERSION "1.0.0"
#endif

//=======================================================================
// function : printUsage
// purpose  : Print usage information
//=======================================================================
void printUsage(const char* programName)
{
  std::cout << "Usage: " << programName << " <input.ezd> <output.step>" << std::endl;
  std::cout << std::endl;
  std::cout << "Convert ezdesign JSON format to STEP file." << std::endl;
  std::cout << std::endl;
  std::cout << "Arguments:" << std::endl;
  std::cout << "  input.ezd    - Input JSON file in ezdesign format" << std::endl;
  std::cout << "  output.step  - Output STEP file" << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  --version, -v  - Show version information" << std::endl;
  std::cout << "  --help, -h      - Show this help message" << std::endl;
  std::cout << std::endl;
  std::cout << "Example:" << std::endl;
  std::cout << "  " << programName << " model.ezd model.step" << std::endl;
}

//=======================================================================
// function : printVersion
// purpose  : Print version information
//=======================================================================
void printVersion()
{
  std::cout << "ezd2step version " << EZD2STEP_VERSION << std::endl;
  std::cout << "Copyright (c) 2025 Yang Song. All rights reserved." << std::endl;
}

//=======================================================================
// function : checkFileExists
// purpose  : Check if file exists and is readable
//=======================================================================
bool checkFileExists(const char* filePath)
{
  std::ifstream file(filePath);
  return file.good();
}

//=======================================================================
// function : checkDirectoryWritable
// purpose  : Check if output directory is writable by attempting to create a test file
//=======================================================================
bool checkDirectoryWritable(const char* filePath)
{
  // Extract directory from file path
  std::string path(filePath);
  size_t lastSlash = path.find_last_of("/\\");
  std::string dir;
  
  if (lastSlash == std::string::npos) {
    // File is in current directory
    dir = ".";
  } else {
    dir = path.substr(0, lastSlash);
  }
  
  // Try to create a temporary file in the directory to test write access
  std::string testFile = dir + "/.ezd2step_write_test";
  std::ofstream test(testFile.c_str());
  if (test.good()) {
    test.close();
    // Remove test file
    std::remove(testFile.c_str());
    return true;
  }
  return false;
}

//=======================================================================
// function : main
// purpose  : Main entry point
//=======================================================================
int main(int argc, char* argv[])
{
  // Handle --version and --help flags
  if (argc == 2) {
    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
      printVersion();
      return 0;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      printUsage(argv[0]);
      return 0;
    }
  }

  if (argc != 3) {
    printUsage(argv[0]);
    return 1;
  }

  const char* inputFile = argv[1];
  const char* outputFile = argv[2];

  try {
    OCC_CATCH_SIGNALS

    // Check input file exists and is readable (exit code 2: file I/O error)
    if (!checkFileExists(inputFile)) {
      std::cerr << "ERROR: Cannot open input file: " << inputFile << std::endl;
      return 2;
    }

    // Check output directory is writable (exit code 2: file I/O error)
    if (!checkDirectoryWritable(outputFile)) {
      std::cerr << "ERROR: Cannot write to output directory for: " << outputFile << std::endl;
      return 2;
    }

    std::cout << "Reading JSON file: " << inputFile << std::endl;

    // 1. Read JSON file (exit code 3: JSON parsing error)
    EzDesignJsonReader reader;
    if (!reader.ReadFile(TCollection_AsciiString(inputFile))) {
      std::cerr << "ERROR: Failed to read JSON file" << std::endl;
      const auto& errors = reader.GetErrors();
      for (const auto& error : errors) {
        std::cerr << "  " << error << std::endl;
      }
      return 3;
    }

    if (!reader.IsDone()) {
      std::cerr << "ERROR: JSON reading incomplete" << std::endl;
      return 3;
    }

    // 2. Validate parsed data (exit code 3: JSON parsing error)
    if (!reader.Validate()) {
      std::cerr << "ERROR: Validation failed" << std::endl;
      const auto& errors = reader.GetErrors();
      for (const auto& error : errors) {
        std::cerr << "  " << error << std::endl;
      }
      return 3;
    }

    std::cout << "Converting to OCCT format..." << std::endl;

    // 3. Convert to OCCT shapes (exit code 4: conversion error)
    EzDesignToOCCTConverter converter(reader);
    const EzBody& body = reader.GetBody();
    TopoDS_Shape shape = converter.ConvertBody(body);

    if (shape.IsNull()) {
      std::cerr << "ERROR: Failed to convert to OCCT shape" << std::endl;
      if (converter.HasErrors()) {
        const auto& errors = converter.GetErrors();
        for (const auto& error : errors) {
          std::cerr << "  " << error << std::endl;
        }
      }
      return 4;
    }

    if (converter.HasErrors()) {
      std::cerr << "WARNING: Conversion completed with errors:" << std::endl;
      const auto& errors = converter.GetErrors();
      for (const auto& error : errors) {
        std::cerr << "  " << error << std::endl;
      }
    }

    std::cout << "Writing STEP file: " << outputFile << std::endl;

    // 4. Export to STEP (exit code 5: STEP export error)
    STEPControl_Writer writer;
    IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);

    if (status != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to transfer shape to STEP format (status: " << status << ")" << std::endl;
      return 5;
    }

    status = writer.Write(outputFile);

    if (status != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to write STEP file (status: " << status << ")" << std::endl;
      return 5;
    }

    // 5. Verify generated STEP file can be read back
    std::cout << "Verifying STEP file..." << std::endl;
    STEPControl_Reader stepReader;
    IFSelect_ReturnStatus readStatus = stepReader.ReadFile(outputFile);

    if (readStatus != IFSelect_RetDone) {
      std::cerr << "ERROR: Generated STEP file cannot be read by OCCT (status: " << readStatus << ")" << std::endl;
      std::cerr << "The STEP file may be invalid or corrupted." << std::endl;
      
      // Print check messages for diagnostics
      stepReader.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
      return 5;
    }

    // Print any check messages (warnings, but file is readable)
    Handle(StepData_StepModel) model = stepReader.StepModel();
    if (!model.IsNull()) {
      Handle(Interface_Check) globalCheck = model->GlobalCheck();
      if (!globalCheck.IsNull() && (globalCheck->HasFailed() || globalCheck->HasWarnings())) {
        std::cerr << "WARNING: STEP file has check messages:" << std::endl;
        stepReader.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
      }
    }

    std::cout << "SUCCESS: STEP file created and verified: " << outputFile << std::endl;
    return 0;
  }
  catch (const Standard_Failure& e) {
    std::cerr << "ERROR: OCCT exception: " << e.GetMessageString() << std::endl;
    // Could be conversion or STEP error, default to conversion error (4)
    return 4;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: Standard exception: " << e.what() << std::endl;
    // Could be file I/O or other, default to file I/O error (2)
    return 2;
  }
  catch (...) {
    std::cerr << "ERROR: Unknown exception occurred" << std::endl;
    // Unknown error, default to conversion error (4)
    return 4;
  }
}

