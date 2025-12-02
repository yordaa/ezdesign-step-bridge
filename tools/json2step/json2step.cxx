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

#include "EzDesignJsonReader.hxx"
#include "EzDesignToOCCTConverter.hxx"

#include <Standard.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <STEPControl_Writer.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TCollection_AsciiString.hxx>
#include <iostream>
#include <cstring>

//=======================================================================
// function : printUsage
// purpose  : Print usage information
//=======================================================================
void printUsage(const char* programName)
{
  std::cout << "Usage: " << programName << " <input.json> <output.step>" << std::endl;
  std::cout << std::endl;
  std::cout << "Convert ezdesign JSON format to STEP file." << std::endl;
  std::cout << std::endl;
  std::cout << "Arguments:" << std::endl;
  std::cout << "  input.json   - Input JSON file in ezdesign format" << std::endl;
  std::cout << "  output.step  - Output STEP file" << std::endl;
  std::cout << std::endl;
  std::cout << "Example:" << std::endl;
  std::cout << "  " << programName << " model.json model.step" << std::endl;
}

//=======================================================================
// function : main
// purpose  : Main entry point
//=======================================================================
int main(int argc, char* argv[])
{
  if (argc != 3) {
    printUsage(argv[0]);
    return 1;
  }

  const char* inputFile = argv[1];
  const char* outputFile = argv[2];

  try {
    OCC_CATCH_SIGNALS

    std::cout << "Reading JSON file: " << inputFile << std::endl;

    // 1. Read JSON file
    EzDesignJsonReader reader;
    if (!reader.ReadFile(TCollection_AsciiString(inputFile))) {
      std::cerr << "ERROR: Failed to read JSON file" << std::endl;
      const auto& errors = reader.GetErrors();
      for (const auto& error : errors) {
        std::cerr << "  " << error << std::endl;
      }
      return 1;
    }

    if (!reader.IsDone()) {
      std::cerr << "ERROR: JSON reading incomplete" << std::endl;
      return 1;
    }

    // 2. Validate parsed data
    if (!reader.Validate()) {
      std::cerr << "ERROR: Validation failed" << std::endl;
      const auto& errors = reader.GetErrors();
      for (const auto& error : errors) {
        std::cerr << "  " << error << std::endl;
      }
      return 1;
    }

    std::cout << "Converting to OCCT format..." << std::endl;

    // 3. Convert to OCCT shapes
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
      return 1;
    }

    if (converter.HasErrors()) {
      std::cerr << "WARNING: Conversion completed with errors:" << std::endl;
      const auto& errors = converter.GetErrors();
      for (const auto& error : errors) {
        std::cerr << "  " << error << std::endl;
      }
    }

    std::cout << "Writing STEP file: " << outputFile << std::endl;

    // 4. Export to STEP
    STEPControl_Writer writer;
    IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);

    if (status != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to transfer shape to STEP format (status: " << status << ")" << std::endl;
      return 1;
    }

    status = writer.Write(outputFile);

    if (status != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to write STEP file (status: " << status << ")" << std::endl;
      return 1;
    }

    std::cout << "SUCCESS: STEP file created: " << outputFile << std::endl;
    return 0;
  }
  catch (const Standard_Failure& e) {
    std::cerr << "ERROR: OCCT exception: " << e.GetMessageString() << std::endl;
    return 1;
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: Standard exception: " << e.what() << std::endl;
    return 1;
  }
  catch (...) {
    std::cerr << "ERROR: Unknown exception occurred" << std::endl;
    return 1;
  }
}

