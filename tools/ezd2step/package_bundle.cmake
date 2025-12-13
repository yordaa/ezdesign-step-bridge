#=======================================================================
# Script: Create ezd2step bundle package
# Purpose:  Package ezd2step executable and required OCCT libraries
# Usage:    cmake -P package_bundle.cmake
#=======================================================================

# Determine platform and architecture
if (APPLE)
  # Detect architecture from OS_WITH_BIT or executable
  if (OS_WITH_BIT MATCHES "mac64" OR OS_WITH_BIT MATCHES "arm64")
    # Check actual executable architecture
    set(EXECUTABLE_PATH_TEMP "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin/ezd2step")
    if (EXISTS "${EXECUTABLE_PATH_TEMP}")
      execute_process(
        COMMAND file "${EXECUTABLE_PATH_TEMP}"
        OUTPUT_VARIABLE FILE_OUTPUT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
      )
      if (FILE_OUTPUT MATCHES "arm64")
        set(PLATFORM "macos-arm64")
      else()
        message(FATAL_ERROR "Only macOS arm64 bundles are supported (found: ${FILE_OUTPUT})")
      endif()
    else()
      # Default to arm64 if we can't check (assume current system)
      set(PLATFORM "macos-arm64")
    endif()
  else()
    message(FATAL_ERROR "Only macOS arm64 bundles are supported")
  endif()
  set(ARCHIVE_SUFFIX ".tar.gz")
  set(EXECUTABLE_NAME "ezd2step")
elseif(WIN32)
  set(PLATFORM "windows-x64")
  set(ARCHIVE_SUFFIX ".zip")
  set(EXECUTABLE_NAME "ezd2step.exe")
else()
  message(FATAL_ERROR "Unsupported platform for bundling")
endif()

# Bundle directory name
set(BUNDLE_NAME "ezd2step-${PROJECT_VERSION}-${PLATFORM}")
set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/bundles/${BUNDLE_NAME}")
set(ARCHIVE_NAME "${BUNDLE_NAME}${ARCHIVE_SUFFIX}")
set(ARCHIVE_PATH "${CMAKE_BINARY_DIR}/bundles/${ARCHIVE_NAME}")

# Create bundle directory
file(MAKE_DIRECTORY "${BUNDLE_DIR}")

# Copy executable
set(EXECUTABLE_PATH "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin/${EXECUTABLE_NAME}")
if (NOT EXISTS "${EXECUTABLE_PATH}")
  message(FATAL_ERROR "Executable not found: ${EXECUTABLE_PATH}")
endif()

# Copy executable (follow symlinks to get actual file)
get_filename_component(EXECUTABLE_REAL "${EXECUTABLE_PATH}" REALPATH)
file(COPY "${EXECUTABLE_REAL}" DESTINATION "${BUNDLE_DIR}")
# Rename to simple name if needed
get_filename_component(EXECUTABLE_REAL_NAME "${EXECUTABLE_REAL}" NAME)
if (NOT EXECUTABLE_REAL_NAME STREQUAL "${EXECUTABLE_NAME}")
  file(RENAME "${BUNDLE_DIR}/${EXECUTABLE_REAL_NAME}" "${BUNDLE_DIR}/${EXECUTABLE_NAME}")
endif()

# Collect and copy OCCT libraries
if (APPLE)
  set(LIB_DIR "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/lib")
  
  # Use otool to get all dependencies
  execute_process(
    COMMAND otool -L "${EXECUTABLE_PATH}"
    OUTPUT_VARIABLE OTOOL_OUTPUT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  # Extract all @rpath libraries (OCCT libraries)
  string(REGEX MATCHALL "@rpath/lib[^ ]+\\.dylib" ALL_LIBS "${OTOOL_OUTPUT}")
  
  # Process each library
  set(COPIED_LIBS "")
  foreach(LIB_REF ${ALL_LIBS})
    # Extract library name (remove @rpath/ prefix)
    string(REGEX REPLACE "@rpath/" "" LIB_NAME "${LIB_REF}")
    
    # Skip system libraries (not in our lib directory)
    if (LIB_NAME MATCHES "^libTK")
      set(LIB_PATH "${LIB_DIR}/${LIB_NAME}")
      if (EXISTS "${LIB_PATH}")
        # Check if already copied
        list(FIND COPIED_LIBS "${LIB_NAME}" ALREADY_COPIED)
        if (ALREADY_COPIED EQUAL -1)
          file(COPY "${LIB_PATH}" DESTINATION "${BUNDLE_DIR}")
          list(APPEND COPIED_LIBS "${LIB_NAME}")
          message(STATUS "Copied library: ${LIB_NAME}")
        endif()
      else()
        message(WARNING "Library not found: ${LIB_PATH}")
      endif()
    endif()
  endforeach()
  
  # Also copy all TK*.dylib files from lib directory (catch any missed dependencies)
  file(GLOB OCCT_DYLIBS "${LIB_DIR}/libTK*.dylib")
  foreach(DYLIB ${OCCT_DYLIBS})
    get_filename_component(DYLIB_NAME "${DYLIB}" NAME)
    list(FIND COPIED_LIBS "${DYLIB_NAME}" ALREADY_COPIED)
    if (ALREADY_COPIED EQUAL -1)
      file(COPY "${DYLIB}" DESTINATION "${BUNDLE_DIR}")
      list(APPEND COPIED_LIBS "${DYLIB_NAME}")
      message(STATUS "Copied library (glob): ${DYLIB_NAME}")
    endif()
  endforeach()
  
  # Update rpath in executable to use @loader_path
  execute_process(
    COMMAND install_name_tool -add_rpath "@loader_path" "${BUNDLE_DIR}/${EXECUTABLE_NAME}"
    RESULT_VARIABLE RPATH_RESULT
    ERROR_QUIET
  )
  if (NOT RPATH_RESULT EQUAL 0)
    message(WARNING "Failed to set rpath in executable (may already be set)")
  endif()
  
elseif(WIN32)
  # Windows: Copy DLLs from bin directory (OCCT DLLs are in bin on Windows)
  set(BIN_DIR "${CMAKE_BINARY_DIR}/${OS_WITH_BIT}/${COMPILER}/bin")
  file(GLOB OCCT_DLLS "${BIN_DIR}/TK*.dll")
  foreach(DLL ${OCCT_DLLS})
    get_filename_component(DLL_NAME "${DLL}" NAME)
    file(COPY "${DLL}" DESTINATION "${BUNDLE_DIR}")
    message(STATUS "Copied DLL: ${DLL_NAME}")
  endforeach()
endif()

# Create README.txt
set(README_CONTENT "ezd2step - EZDesign to STEP Converter
Version: ${PROJECT_VERSION}
Platform: ${PLATFORM}

USAGE:
  ./ezd2step <input.ezd> <output.step>

  Convert EZDesign JSON format (.ezd) to STEP file format.

OPTIONS:
  --version, -v    Show version information
  --help, -h       Show help message

EXAMPLES:
  ./ezd2step model.ezd model.step

LIBRARY PATH OVERRIDE:
  To use custom OCCT libraries (for LGPL compliance), set environment variable:
  
  macOS:   export DYLD_LIBRARY_PATH=/path/to/custom/libs:\$DYLD_LIBRARY_PATH
  Windows: set PATH=C:\\path\\to\\custom\\libs;%PATH%

  The tool will search the override path before bundled libraries.

LICENSE:
  This software uses Open CASCADE Technology (OCCT), licensed under LGPL 2.1.
  See LICENSE.txt for full license text.

  OCCT version: ${OCC_VERSION_MAJOR}.${OCC_VERSION_MINOR}.${OCC_VERSION_MAINTENANCE}
  OCCT source and build instructions: See source package

For more information, visit: https://github.com/your-repo/ezdesign-step-bridge
")
file(WRITE "${BUNDLE_DIR}/README.txt" "${README_CONTENT}")

# Create LICENSE.txt (LGPL 2.1)
# Note: This is a placeholder - should include full LGPL 2.1 text
set(LICENSE_CONTENT "GNU LESSER GENERAL PUBLIC LICENSE
Version 2.1, February 1999

This software uses Open CASCADE Technology (OCCT), which is licensed under
the GNU Lesser General Public License version 2.1.

For the full text of the LGPL 2.1 license, please visit:
https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html

OCCT Version: ${OCC_VERSION_MAJOR}.${OCC_VERSION_MINOR}.${OCC_VERSION_MAINTENANCE}

Source code and build instructions for OCCT are provided in the source package.
")
file(WRITE "${BUNDLE_DIR}/LICENSE.txt" "${LICENSE_CONTENT}")

# Create archive
if (APPLE)
  execute_process(
    COMMAND tar -czf "${ARCHIVE_PATH}" -C "${CMAKE_BINARY_DIR}/bundles" "${BUNDLE_NAME}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bundles"
    RESULT_VARIABLE TAR_RESULT
  )
  if (NOT TAR_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to create tar archive")
  endif()
elseif(WIN32)
  # Windows: Use CMake's built-in zip or external tool
  # For now, create a note that zip should be created manually or via CI
  message(STATUS "Windows bundle created at: ${BUNDLE_DIR}")
  message(STATUS "Create ZIP archive manually or via CI: ${ARCHIVE_PATH}")
endif()

message(STATUS "Bundle created: ${BUNDLE_DIR}")
if (EXISTS "${ARCHIVE_PATH}")
  message(STATUS "Archive created: ${ARCHIVE_PATH}")
endif()
