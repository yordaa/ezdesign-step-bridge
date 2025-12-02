#!/bin/bash
# Command to compile and run the minimal STEP export test
# This script assumes OCCT has been built and installed

OCCT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${OCCT_ROOT}/build"
INSTALL_DIR="${OCCT_ROOT}/install"

# Try to find libraries and headers
if [ -d "${INSTALL_DIR}/lib" ] && [ -d "${INSTALL_DIR}/include/opencascade" ]; then
  # Use install directory
  INC_DIR="${INSTALL_DIR}/include"
  LIB_DIR="${INSTALL_DIR}/lib"
  echo "Using install directory: ${INSTALL_DIR}"
elif [ -d "${BUILD_DIR}/mac64/clang/lib" ] && [ -d "${BUILD_DIR}/include/opencascade" ]; then
  # Use build directory (macOS)
  INC_DIR="${BUILD_DIR}/include"
  LIB_DIR="${BUILD_DIR}/mac64/clang/lib"
  echo "Using build directory: ${BUILD_DIR}"
else
  echo "Error: Could not find OCCT libraries and headers"
  echo "Please build and install OCCT first, or set OCCT_ROOT environment variable"
  exit 1
fi

echo "Compiling test program..."
echo "Include directory: ${INC_DIR}"
echo "Library directory: ${LIB_DIR}"

# Compile
clang++ -std=c++11 \
  -I"${INC_DIR}" \
  -L"${LIB_DIR}" \
  -Wl,-rpath,"${LIB_DIR}" \
  "${OCCT_ROOT}/adm/cmake/test_minimal_step_export.cxx" \
  -lTKDESTEP -lTKSTEPBase -lTKSTEPAttr -lTKSTEP209 -lTKSTEP \
  -lTKXSBase -lTKDE -lTKTopAlgo -lTKGeomAlgo -lTKGeomBase \
  -lTKBRep -lTKG3d -lTKG2d -lTKMath -lTKernel \
  -o test_minimal_step_export

# Run
if [ -f ./test_minimal_step_export ]; then
  echo ""
  echo "Running test program..."
  ./test_minimal_step_export test_minimal_step_export.step
  echo ""
  if [ -f test_minimal_step_export.step ]; then
    echo "SUCCESS: STEP file created: test_minimal_step_export.step"
    echo "File size: $(wc -c < test_minimal_step_export.step) bytes"
  fi
else
  echo "Compilation failed"
  exit 1
fi

