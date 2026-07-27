cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0207)
  cmake_policy(SET CMP0207 NEW)
endif()

if(NOT EXISTS "${EZD2STEP_EXECUTABLE}")
  message(FATAL_ERROR "Missing ezd2step executable: ${EZD2STEP_EXECUTABLE}")
endif()

if(APPLE)
  set(platform macos-arm64)
  set(archive_extension tar.gz)
  set(archive_format gnutar)
elseif(WIN32)
  set(platform windows-x64)
  set(archive_extension zip)
  set(archive_format zip)
else()
  message(FATAL_ERROR "Bundles support only macOS and Windows")
endif()

set(bundle_name "ezd2step-${EZD2STEP_VERSION}-${platform}")
set(bundle_dir "${EZD2STEP_BINARY_DIR}/bundles/${bundle_name}")
set(archive "${EZD2STEP_BINARY_DIR}/bundles/${bundle_name}.${archive_extension}")
file(REMOVE_RECURSE "${bundle_dir}")
file(MAKE_DIRECTORY "${bundle_dir}")
file(COPY "${EZD2STEP_EXECUTABLE}" DESTINATION "${bundle_dir}")

file(GET_RUNTIME_DEPENDENCIES
  EXECUTABLES "${EZD2STEP_EXECUTABLE}"
  RESOLVED_DEPENDENCIES_VAR dependencies
  UNRESOLVED_DEPENDENCIES_VAR unresolved
  PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-"
  POST_EXCLUDE_REGEXES
    "^/System/"
    "^/usr/lib/"
    "[/\\\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\\\]"
)
if(unresolved)
  message(FATAL_ERROR "Unresolved runtime dependencies: ${unresolved}")
endif()
foreach(dependency IN LISTS dependencies)
  file(COPY "${dependency}" DESTINATION "${bundle_dir}")
endforeach()

configure_file(
  "${EZD2STEP_SOURCE_DIR}/LICENSE"
  "${bundle_dir}/LICENSE.txt"
  COPYONLY
)
file(COPY "${EZD2STEP_SOURCE_DIR}/NOTICE" DESTINATION "${bundle_dir}")
file(WRITE "${bundle_dir}/README.txt"
  "ezd2step ${EZD2STEP_VERSION}\n\nUsage: ezd2step <input.ezd> <output.step>\n")

file(REMOVE "${archive}")
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar cf "${archive}"
          --format=${archive_format} "${bundle_name}"
  WORKING_DIRECTORY "${EZD2STEP_BINARY_DIR}/bundles"
  COMMAND_ERROR_IS_FATAL ANY
)
