//=======================================================================
// File: ezd_to_step.cxx
// Purpose: C API implementation for EZDesign to STEP conversion
// Copyright (c) 2025 Yang Song. All rights reserved.
//=======================================================================

#include "ezd_to_step.h"
#include "EzDesignJsonReader.hxx"
#include "EzDesignToOCCTConverter.hxx"
#include <Standard.hxx>
#include <Standard_Failure.hxx>
#include <Standard_ErrorHandler.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPControl_Reader.hxx>
#include <StepData_StepModel.hxx>
#include <Interface_Check.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TCollection_AsciiString.hxx>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

// Helper function to check if a file exists
static bool checkFileExists(const char* path)
{
  std::ifstream file(path);
  return file.good();
}

// Helper function to check if a directory is writable
static bool checkDirectoryWritable(const std::string& path)
{
  if (path.empty()) return false;
  std::string testFilePath = path + "/.test_writable";
  std::ofstream testFile(testFilePath);
  if (testFile.is_open()) {
    testFile.close();
    std::remove(testFilePath.c_str());
    return true;
  }
  return false;
}

// Helper function to write log message
static void writeLog(const struct ezd_to_step_options* options, 
                     const std::string& message, 
                     bool isError = false)
{
  if (!options || !options->log_file) {
    // No log file, output to stderr for errors, stdout for info
    if (isError) {
      std::cerr << message;
    } else {
      std::cout << message;
    }
    return;
  }

  // Write to log file
  std::ofstream logFile(options->log_file, std::ios::app);
  if (logFile.is_open()) {
    logFile << message;
    logFile.close();
  }
}

// Helper function to write progress message
static void writeProgress(const struct ezd_to_step_options* options, 
                          const std::string& message)
{
  if (!options || options->verbose >= 1) {
    writeLog(options, message, false);
  }
}

// Helper function to write debug message
static void writeDebug(const struct ezd_to_step_options* options, 
                       const std::string& message)
{
  if (options && options->verbose >= 2) {
    writeLog(options, "[DEBUG] " + message, false);
  }
}

// Helper function to write error message
static void writeError(const struct ezd_to_step_options* options, 
                       const std::string& message)
{
  writeLog(options, "ERROR: " + message + "\n", true);
}

extern "C" {

int ezd_to_step(const char* input_path, const char* output_path, 
                const struct ezd_to_step_options* options)
{
  // Default options
  struct ezd_to_step_options defaultOptions = {1, NULL};
  const struct ezd_to_step_options* opts = options ? options : &defaultOptions;

  // Validate arguments
  if (!input_path || !output_path) {
    writeError(opts, "Invalid arguments: input_path and output_path must not be NULL");
    return 1; // Invalid arguments
  }

  try {
    OCC_CATCH_SIGNALS

    // Pre-check input file existence
    if (!checkFileExists(input_path)) {
      writeError(opts, std::string("Cannot open input file: ") + input_path);
      return 2; // File I/O error
    }

    // Pre-check output directory writability
    std::string outputPathStr(output_path);
    size_t lastSlash = outputPathStr.find_last_of("/\\");
    std::string outputDir = (lastSlash == std::string::npos) ? "." : outputPathStr.substr(0, lastSlash);
    if (!checkDirectoryWritable(outputDir)) {
      writeError(opts, std::string("Output directory not writable: ") + outputDir);
      return 2; // File I/O error
    }

    writeProgress(opts, std::string("Reading EZDesign JSON file: ") + input_path + "\n");

    // 1. Read JSON file
    EzDesignJsonReader reader;
    if (!reader.ReadFile(TCollection_AsciiString(input_path))) {
      writeError(opts, "Failed to read JSON file");
      const auto& errors = reader.GetErrors();
      for (const auto& error : errors) {
        writeError(opts, "  " + error);
      }
      return 3; // JSON parsing error
    }

    if (!reader.IsDone()) {
      writeError(opts, "JSON reading incomplete");
      return 3; // JSON parsing error
    }

    // 2. Validate parsed data
    if (!reader.Validate()) {
      writeError(opts, "Validation failed");
      const auto& errors = reader.GetErrors();
      for (const auto& error : errors) {
        writeError(opts, "  " + error);
      }
      return 3; // JSON parsing error
    }

    writeProgress(opts, "Converting to OCCT format...\n");

    // 3. Convert to OCCT shapes
    EzDesignToOCCTConverter converter(reader);
    const EzBody& body = reader.GetBody();
    TopoDS_Shape shape = converter.ConvertBody(body);

    if (shape.IsNull()) {
      writeError(opts, "Failed to convert to OCCT shape");
      if (converter.HasErrors()) {
        const auto& errors = converter.GetErrors();
        for (const auto& error : errors) {
          writeError(opts, "  " + error);
        }
      }
      return 4; // Geometry conversion error
    }

    if (converter.HasErrors()) {
      writeDebug(opts, "Conversion completed with warnings:\n");
      const auto& errors = converter.GetErrors();
      for (const auto& error : errors) {
        writeDebug(opts, "  " + error + "\n");
      }
    }

    writeProgress(opts, std::string("Writing STEP file: ") + output_path + "\n");

    // 4. Export to STEP
    STEPControl_Writer writer;
    IFSelect_ReturnStatus status = writer.Transfer(shape, STEPControl_AsIs);

    if (status != IFSelect_RetDone) {
      writeError(opts, std::string("Failed to transfer shape to STEP format (status: ") + 
                 std::to_string(status) + ")");
      return 5; // STEP export error
    }

    status = writer.Write(output_path);

    if (status != IFSelect_RetDone) {
      writeError(opts, std::string("Failed to write STEP file (status: ") + 
                 std::to_string(status) + ")");
      return 5; // STEP export error
    }

    writeProgress(opts, std::string("SUCCESS: STEP file created: ") + output_path + "\n");

    // 5. Verify generated STEP file
    writeDebug(opts, std::string("Verifying generated STEP file: ") + output_path + "\n");
    STEPControl_Reader stepReader;
    IFSelect_ReturnStatus readStatus = stepReader.ReadFile(output_path);

    if (readStatus != IFSelect_RetDone) {
      writeError(opts, "Generated STEP file is invalid and cannot be read by OCCT.");
      writeError(opts, std::string("Read status: ") + std::to_string(readStatus));
      stepReader.PrintCheckLoad(Standard_False, IFSelect_ItemsByEntity);
      return 5; // STEP export error (verification failed)
    }

    writeDebug(opts, "Generated STEP file verified successfully by OCCT.\n");
    return 0; // Success
  }
  catch (const Standard_Failure& e) {
    writeError(opts, std::string("OCCT exception: ") + e.GetMessageString());
    return 4; // Geometry conversion error (or general OCCT error)
  }
  catch (const std::exception& e) {
    writeError(opts, std::string("Standard exception: ") + e.what());
    return 4; // Geometry conversion error (or general exception)
  }
  catch (...) {
    writeError(opts, "Unknown exception occurred");
    return 4; // Geometry conversion error (or unknown exception)
  }
}

} // extern "C"

