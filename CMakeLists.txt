cmake_minimum_required(VERSION 3.20)
project(ezd2step VERSION 1.0.2 LANGUAGES CXX)

include(CTest)
find_package(OpenCASCADE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(EZD2STEP_SOURCES
  src/ezd_to_step.cxx
  src/EzDesignJsonReader.cxx
  src/EzDesignToOCCTConverter.cxx
)
set(EZD2STEP_LIBRARIES
  nlohmann_json::nlohmann_json
  TKDESTEP TKXSBase TKDE TKBRep TKTopAlgo TKGeomAlgo
  TKGeomBase TKG3d TKG2d TKMath TKernel
)

function(ezd2step_target target)
  target_compile_features(${target} PRIVATE cxx_std_17)
  target_include_directories(${target} PRIVATE src include)
  target_link_libraries(${target} PRIVATE ${EZD2STEP_LIBRARIES})
endfunction()

add_executable(ezd2step src/ezd2step.cxx ${EZD2STEP_SOURCES})
ezd2step_target(ezd2step)
target_compile_definitions(ezd2step PRIVATE
  EZD2STEP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  EZD2STEP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  EZD2STEP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  EZD2STEP_VERSION="${PROJECT_VERSION}"
)

add_library(ezd2step_capi SHARED ${EZD2STEP_SOURCES})
ezd2step_target(ezd2step_capi)
set_target_properties(ezd2step_capi PROPERTIES
  OUTPUT_NAME ezd2step
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(APPLE)
  set_target_properties(ezd2step ezd2step_capi PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH FALSE
  )
endif()

install(TARGETS ezd2step ezd2step_capi
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES include/ezd_to_step.h DESTINATION include)

if(BUILD_TESTING)
  add_executable(test_json_reader_entities_schema
    tests/test_json_reader_entities_schema.cxx
    src/EzDesignJsonReader.cxx
  )
  ezd2step_target(test_json_reader_entities_schema)

  add_executable(test_20260427_conversion
    tests/test_20260427_conversion.cxx
    ${EZD2STEP_SOURCES}
  )
  ezd2step_target(test_20260427_conversion)

  add_test(NAME json_reader_entities_schema
    COMMAND test_json_reader_entities_schema
  )
  add_test(NAME conversion_20260427
    COMMAND test_20260427_conversion
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/20260427.ezd
  )
endif()

add_custom_target(package_ezd2step_bundle
  COMMAND ${CMAKE_COMMAND}
    -DEZD2STEP_EXECUTABLE=$<TARGET_FILE:ezd2step>
    -DEZD2STEP_VERSION=${PROJECT_VERSION}
    -DEZD2STEP_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DEZD2STEP_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/package_bundle.cmake
  DEPENDS ezd2step
  VERBATIM
)
