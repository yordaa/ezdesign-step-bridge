// Test program to verify STEP export functionality works with minimal builds
// This program tests that STEPControl_Writer can be instantiated and used
// to export a simple shape to STEP format.

#include <STEPControl_Writer.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopoDS_Shape.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Standard.hxx>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
  const char* outputFile = (argc > 1) ? argv[1] : "test_minimal_step_export.step";
  
  std::cout << "Testing STEP export with minimal build..." << std::endl;
  
  try {
    // Test 1: Create a simple box shape
    std::cout << "Creating a simple box shape..." << std::endl;
    BRepPrimAPI_MakeBox boxMaker(10.0, 20.0, 30.0);
    TopoDS_Shape box = boxMaker.Shape();
    
    if (box.IsNull()) {
      std::cerr << "ERROR: Failed to create box shape" << std::endl;
      return 1;
    }
    std::cout << "  Box shape created successfully" << std::endl;
    
    // Test 2: Instantiate STEPControl_Writer
    std::cout << "Instantiating STEPControl_Writer..." << std::endl;
    STEPControl_Writer writer;
    
    if (writer.WS().IsNull()) {
      std::cerr << "ERROR: STEPControl_Writer session is null" << std::endl;
      return 1;
    }
    std::cout << "  STEPControl_Writer instantiated successfully" << std::endl;
    
    // Test 3: Transfer shape to STEP
    std::cout << "Transferring shape to STEP format..." << std::endl;
    IFSelect_ReturnStatus transferStatus = writer.Transfer(box, STEPControl_AsIs);
    
    if (transferStatus != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to transfer shape to STEP (status: " << transferStatus << ")" << std::endl;
      return 1;
    }
    std::cout << "  Shape transferred successfully" << std::endl;
    
    // Test 4: Write STEP file
    std::cout << "Writing STEP file: " << outputFile << "..." << std::endl;
    IFSelect_ReturnStatus writeStatus = writer.Write(outputFile);
    
    if (writeStatus != IFSelect_RetDone) {
      std::cerr << "ERROR: Failed to write STEP file (status: " << writeStatus << ")" << std::endl;
      return 1;
    }
    std::cout << "  STEP file written successfully" << std::endl;
    
    // Test 5: Verify file exists and is not empty
    std::ifstream fileCheck(outputFile);
    if (!fileCheck.good()) {
      std::cerr << "ERROR: Output file does not exist or cannot be read" << std::endl;
      return 1;
    }
    
    fileCheck.seekg(0, std::ios::end);
    std::streampos fileSize = fileCheck.tellg();
    if (fileSize <= 0) {
      std::cerr << "ERROR: Output file is empty" << std::endl;
      return 1;
    }
    
    std::cout << "  STEP file verified: " << fileSize << " bytes" << std::endl;
    
    std::cout << std::endl << "SUCCESS: All STEP export tests passed!" << std::endl;
    std::cout << "Minimal build STEP export functionality is working correctly." << std::endl;
    
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

