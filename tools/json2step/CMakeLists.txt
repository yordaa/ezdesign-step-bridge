project(json2step)

# Detect nlohmann/json
find_package(nlohmann_json QUIET)
if (nlohmann_json_FOUND)
  message(STATUS "Found nlohmann/json via find_package")
else()
  # Try Homebrew paths on macOS
  if (APPLE)
    if (EXISTS "/opt/homebrew/include/nlohmann/json.hpp")
      set(nlohmann_json_INCLUDE_DIR "/opt/homebrew/include")
      message(STATUS "Found nlohmann/json in Homebrew: ${nlohmann_json_INCLUDE_DIR}")
    elseif (EXISTS "/usr/local/include/nlohmann/json.hpp")
      set(nlohmann_json_INCLUDE_DIR "/usr/local/include")
      message(STATUS "Found nlohmann/json in Homebrew: ${nlohmann_json_INCLUDE_DIR}")
    endif()
  endif()
  
  if (NOT nlohmann_json_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json not found. Please install it via:\n  macOS: brew install nlohmann-json\n  Linux: apt-get install nlohmann-json-dev (or equivalent)\n  Windows: vcpkg install nlohmann-json")
  endif()
endif()

# Include OCCT build system
OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit_prepare_tool)
set (EXECUTABLE_PROJECT ON)
# Disable Qt for this tool (json2step doesn't need Qt)
set (USE_QT OFF)
OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)
unset (EXECUTABLE_PROJECT)
unset (USE_QT)

# Add nlohmann/json include directory
# Note: This must be after occt_toolkit.cmake creates the target
if (nlohmann_json_FOUND)
  # When found via find_package, use the imported target or INCLUDE_DIRS
  if (TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
  else()
    include_directories(${nlohmann_json_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${nlohmann_json_INCLUDE_DIRS})
  endif()
elseif(nlohmann_json_INCLUDE_DIR)
  include_directories(${nlohmann_json_INCLUDE_DIR})
  target_include_directories(${PROJECT_NAME} PRIVATE ${nlohmann_json_INCLUDE_DIR})
endif()

# Required OCCT toolkits for STEP export
# These will be automatically resolved via EXTERNLIB file

# Round-trip test program
# Use same approach as json2step - standalone executable with manual library linking
add_executable(test_step_roundtrip test_step_roundtrip.cxx)
# Link against required OCCT libraries for STEP reading
# Note: Using library names without TK prefix (OCCT internal naming)
target_link_libraries(test_step_roundtrip PRIVATE
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKSTEPControl${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKSTEP${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKXSBase${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKDE${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKBRep${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKTopAlgo${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKGeomBase${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKGeomAlgo${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKG3d${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKG2d${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKMath${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKernel${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKDESTEP${CMAKE_STATIC_LIBRARY_SUFFIX}
  ${CMAKE_BINARY_DIR}/mac64/clang/lib/${CMAKE_STATIC_LIBRARY_PREFIX}TKCDF${CMAKE_STATIC_LIBRARY_SUFFIX}
)

