cmake_minimum_required(VERSION 3.20)
project(ezdesign-step-bridge VERSION 1.0.3 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)
find_package(OpenCASCADE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

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

add_executable(ezd2step
  src/ezd2step.cxx
  src/EzDesignJsonReader.cxx
  src/EzDesignToOCCTConverter.cxx
)
ezd2step_target(ezd2step)
target_compile_definitions(ezd2step PRIVATE
  EZD2STEP_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  EZD2STEP_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  EZD2STEP_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  EZD2STEP_VERSION="${PROJECT_VERSION}"
)

if(APPLE)
  set_target_properties(ezd2step PROPERTIES
    INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
    BUILD_WITH_INSTALL_RPATH FALSE
  )
endif()

install(TARGETS ezd2step RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES LICENSE NOTICE DESTINATION ${CMAKE_INSTALL_DATADIR}/ezdesign-step-bridge)

if(BUILD_TESTING)
  add_executable(test_json_reader_entities_schema
    tests/test_json_reader_entities_schema.cxx
    src/EzDesignJsonReader.cxx
  )
  ezd2step_target(test_json_reader_entities_schema)

  add_executable(test_20260427_conversion
    tests/test_20260427_conversion.cxx
  )
  ezd2step_target(test_20260427_conversion)

  add_test(NAME json_reader_entities_schema
    COMMAND test_json_reader_entities_schema
  )
  set(EZD2STEP_TEST_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/20260427.step")
  add_test(NAME installed_cli_conversion_20260427
    COMMAND "${CMAKE_INSTALL_FULL_BINDIR}/ezd2step${CMAKE_EXECUTABLE_SUFFIX}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/data/20260427.ezd"
            "${EZD2STEP_TEST_OUTPUT}"
  )
  add_test(NAME conversion_20260427_geometry
    COMMAND test_20260427_conversion
            "${EZD2STEP_TEST_OUTPUT}"
  )
  set_tests_properties(conversion_20260427_geometry PROPERTIES
    DEPENDS installed_cli_conversion_20260427
  )
endif()
