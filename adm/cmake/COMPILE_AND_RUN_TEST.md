# How to Compile and Run the Minimal STEP Export Test

## Command to Compile and Run

If OCCT has been built (not necessarily installed), use this command from the OCCT root directory:

```bash
# From OCCT root directory
cd /Users/songyang/Codes/modeling-refs/OCCT

# Compile
clang++ -std=c++11 \
  -I./src \
  -L./build/mac64/clang/lib \
  -Wl,-rpath,./build/mac64/clang/lib \
  ./adm/cmake/test_minimal_step_export.cxx \
  -lTKDESTEP -lTKSTEPBase -lTKSTEPAttr -lTKSTEP209 -lTKSTEP \
  -lTKXSBase -lTKDE -lTKTopAlgo -lTKGeomAlgo -lTKGeomBase \
  -lTKBRep -lTKG3d -lTKG2d -lTKMath -lTKernel \
  -o test_minimal_step_export

# Run
./test_minimal_step_export test_minimal_step_export.step
```

## Alternative: Using CMake (Recommended)

If you have a minimal build configured, you can add the test to your CMake build:

```cmake
# In your CMakeLists.txt or build directory
add_executable(test_minimal_step_export
  ${CMAKE_SOURCE_DIR}/adm/cmake/test_minimal_step_export.cxx
)

target_link_libraries(test_minimal_step_export
  OpenCASCADE::TKDESTEP
  OpenCASCADE::TKSTEPBase
  OpenCASCADE::TKSTEPAttr
  OpenCASCADE::TKSTEP209
  OpenCASCADE::TKSTEP
  OpenCASCADE::TKXSBase
  OpenCASCADE::TKDE
  OpenCASCADE::TKTopAlgo
  OpenCASCADE::TKGeomAlgo
  OpenCASCADE::TKGeomBase
  OpenCASCADE::TKBRep
  OpenCASCADE::TKG3d
  OpenCASCADE::TKG2d
  OpenCASCADE::TKMath
  OpenCASCADE::TKernel
)
```

Then build and run:
```bash
cmake --build . --target test_minimal_step_export
./test_minimal_step_export test_minimal_step_export.step
```

