project(ezd2step VERSION 1.0.2)

# Find nlohmann/json (uses vcpkg manifest mode when vcpkg toolchain is used)
find_package(nlohmann_json REQUIRED)

set(EZD2STEP_CONVERTER_SOURCES
  ezd_to_step.cxx
  EzDesignJsonReader.cxx
  EzDesignToOCCTConverter.cxx
)

set(EZD2STEP_OCCT_LIBS
  TKDESTEP
  TKXSBase
  TKDE
  TKBRep
  TKTopAlgo
  TKGeomAlgo
  TKGeomBase
  TKG3d
  TKG2d
  TKMath
  TKernel
  TKCDF
)

function(ezd2step_enable_cxx17 target_name)
  set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
  )
  target_compile_features(${target_name} PRIVATE cxx_std_17)
endfunction()

# Include OCCT build system
OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit_prepare_tool)
set (EXECUTABLE_PROJECT ON)
# Disable unnecessary modules for ezd2step (command-line tool, no GUI)
set (USE_QT OFF)
set (USE_TK OFF)
set (USE_TCL OFF)
set (BUILD_MODULE_Draw OFF)
set (USE_FREETYPE OFF)
OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)
unset (EXECUTABLE_PROJECT)
unset (USE_QT)
unset (USE_TK)
unset (USE_TCL)
unset (BUILD_MODULE_Draw)
unset (USE_FREETYPE)

# Link against nlohmann/json
# Note: This must be after occt_toolkit.cmake creates the target
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Require C++17 for structured bindings and other modern features
set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

# Define version for ezd2step
target_compile_definitions(${PROJECT_NAME} PRIVATE
  EZD2STEP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  EZD2STEP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  EZD2STEP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  EZD2STEP_VERSION="${PROJECT_VERSION}"
)

# Configure runtime library paths for bundling
# macOS: Use @loader_path to locate libraries in same directory as executable
if (APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
    MACOSX_RPATH ON
  )
  # Also set rpath for development builds
  set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_RPATH "@loader_path"
  )
endif()

# Windows: DLLs in same directory as executable (default search path)
# No special configuration needed - Windows searches executable directory by default

# C API Shared Library
# Create shared library for C API (libezd2step.dylib on macOS, ezd2step.dll on Windows)
add_library(ezd2step_capi SHARED
  ${EZD2STEP_CONVERTER_SOURCES}
)

# Set library name
set_target_properties(ezd2step_capi PROPERTIES
  OUTPUT_NAME "ezd2step"
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)
ezd2step_enable_cxx17(ezd2step_capi)

# Link against nlohmann/json
target_link_libraries(ezd2step_capi PRIVATE nlohmann_json::nlohmann_json)

# Link against OCCT libraries (same as executable)
# Link against OCCT library targets
target_link_libraries(ezd2step_capi PRIVATE
  ${EZD2STEP_OCCT_LIBS}
)

# Configure rpath for shared library (same as executable)
if (APPLE)
  set_target_properties(ezd2step_capi PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
    MACOSX_RPATH ON
    BUILD_RPATH "@loader_path"
  )
endif()

# Install C API header
install(FILES ezd_to_step.h DESTINATION include)

# Install shared library
install(TARGETS ezd2step_capi
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

# Create custom target for bundle packaging
# This will be called after build completes
add_custom_target(package_ezd2step_bundle
  COMMAND ${CMAKE_COMMAND}
    -DCMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}
    -DOS_WITH_BIT=${OS_WITH_BIT}
    -DCOMPILER=${COMPILER}
    -DPROJECT_VERSION=${PROJECT_VERSION}
    -DOCC_VERSION_MAJOR=${OCC_VERSION_MAJOR}
    -DOCC_VERSION_MINOR=${OCC_VERSION_MINOR}
    -DOCC_VERSION_MAINTENANCE=${OCC_VERSION_MAINTENANCE}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/package_bundle.cmake
  DEPENDS ${PROJECT_NAME}
  COMMENT "Creating ezd2step bundle package"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Required OCCT toolkits for STEP export
# These will be automatically resolved via EXTERNLIB file

# Round-trip test program
# Use same approach as ezd2step - standalone executable with manual library linking
add_executable(test_step_roundtrip test_step_roundtrip.cxx)
# Link against required OCCT libraries for STEP reading
# Note: Using library names without TK prefix (OCCT internal naming)
# Link against shared libraries (.dylib on macOS)
target_link_libraries(test_step_roundtrip PRIVATE
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKDESTEP.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKSTEP.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKXSBase.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKDE.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKBRep.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKTopAlgo.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKGeomBase.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKGeomAlgo.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKG3d.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKG2d.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKMath.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKernel.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKCDF.dylib
)

# Basic models test program
# Use same approach as test_step_roundtrip - standalone executable with manual library linking
add_executable(test_basic_models test_basic_models.cxx)
# Link against required OCCT libraries for STEP reading
# Note: Using library names without TK prefix (OCCT internal naming)
# Link against shared libraries (.dylib on macOS)
target_link_libraries(test_basic_models PRIVATE
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKDESTEP.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKXSBase.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKDE.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKBRep.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKTopAlgo.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKGeomBase.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKGeomAlgo.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKG3d.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKG2d.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKMath.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKernel.dylib
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/libTKCDF.dylib
)

add_executable(test_20260427_conversion
  test_20260427_conversion.cxx
  ${EZD2STEP_CONVERTER_SOURCES}
)
ezd2step_enable_cxx17(test_20260427_conversion)
target_link_libraries(test_20260427_conversion PRIVATE
  nlohmann_json::nlohmann_json
  ${EZD2STEP_OCCT_LIBS}
)
add_custom_command(TARGET test_20260427_conversion POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/20260427.ezd
          $<TARGET_FILE_DIR:test_20260427_conversion>/20260427.ezd
)

add_executable(test_json_reader_entities_schema
  test_json_reader_entities_schema.cxx
  EzDesignJsonReader.cxx
)
ezd2step_enable_cxx17(test_json_reader_entities_schema)
target_link_libraries(test_json_reader_entities_schema PRIVATE
  nlohmann_json::nlohmann_json
  TKernel
)

