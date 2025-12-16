# Portfile for ezd2step - EZDesign to STEP converter
# Downloads pre-built bundle from GitHub Releases

# Version is defined in vcpkg.json and available as ${VERSION}
# For v1.0.0, VERSION will be "1.0.0"

# Detect platform using vcpkg variables (not APPLE/WIN32 which don't work in script mode)
if(VCPKG_TARGET_IS_OSX AND VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
    set(PLATFORM "macos-arm64")
    set(ARCHIVE_SUFFIX ".tar.gz")
    set(ARCHIVE_NAME "ezd2step-${VERSION}-macos-arm64.tar.gz")
    # SHA512 hash for macOS arm64 bundle (v1.0.0)
    set(ARCHIVE_SHA512 "f8b856f56cac6b09169d7170f47045b17e5663a7b137d5b8bc484297e4e84bbc038a1945e6a2a15eecb315b90f4b2a25e6420d1d9f91433b77b96cdb19c86062")
elseif(VCPKG_TARGET_IS_WINDOWS AND VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    set(PLATFORM "windows-x64")
    set(ARCHIVE_SUFFIX ".zip")
    set(ARCHIVE_NAME "ezd2step-${VERSION}-windows-x64.zip")
    # TODO: Add SHA512 hash for Windows x64 bundle when available
    set(ARCHIVE_SHA512 "")
    message(FATAL_ERROR "Windows x64 bundle not yet available. Only macOS arm64 is supported in v1.0.0")
else()
    message(FATAL_ERROR "Unsupported platform: ${VCPKG_CMAKE_SYSTEM_NAME} ${VCPKG_TARGET_ARCHITECTURE}. Supported: macOS arm64")
endif()

# Download bundle from GitHub Releases
# For private repos, use gh CLI which handles authentication automatically
find_program(GH_CLI gh)
if(GH_CLI)
    message(STATUS "Downloading ${ARCHIVE_NAME} using gh CLI (private repo)...")
    # Use vcpkg's downloads directory for consistency
    set(DOWNLOAD_DIR "${DOWNLOADS}")
    set(ARCHIVE "${DOWNLOAD_DIR}/${ARCHIVE_NAME}")
    
    # Use gh CLI to download from private release
    execute_process(
        COMMAND ${GH_CLI} release download v${VERSION}
            --repo yordaa/ezdesign-step-bridge
            --pattern "${ARCHIVE_NAME}"
            --dir "${DOWNLOAD_DIR}"
        RESULT_VARIABLE GH_RESULT
        ERROR_VARIABLE GH_ERROR
        OUTPUT_VARIABLE GH_OUTPUT
    )
    
    if(NOT GH_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to download ${ARCHIVE_NAME} using gh CLI: ${GH_ERROR}")
    endif()
    
    # Verify file exists and check SHA512
    if(NOT EXISTS "${ARCHIVE}")
        message(FATAL_ERROR "Downloaded file not found: ${ARCHIVE}")
    endif()
    
    # Verify SHA512 hash
    file(SHA512 "${ARCHIVE}" DOWNLOADED_SHA512)
    if(NOT DOWNLOADED_SHA512 STREQUAL ARCHIVE_SHA512)
        message(FATAL_ERROR "SHA512 mismatch for ${ARCHIVE_NAME}. Expected: ${ARCHIVE_SHA512}, Got: ${DOWNLOADED_SHA512}")
    endif()
    
    message(STATUS "Downloaded and verified ${ARCHIVE_NAME}")
else()
    # Fallback to vcpkg_download_distfile (may fail for private repos)
    message(STATUS "gh CLI not found, using vcpkg_download_distfile (may fail for private repos)...")
    vcpkg_download_distfile(
        ARCHIVE
        URLS "https://github.com/yordaa/ezdesign-step-bridge/releases/download/v${VERSION}/${ARCHIVE_NAME}"
        FILENAME "${ARCHIVE_NAME}"
        SHA512 "${ARCHIVE_SHA512}"
    )
endif()

# Extract archive
# When using gh CLI, we have the file in DOWNLOADS, so we can use vcpkg_extract_source_archive
# But we need to pass just the filename, not the full path, or extract manually
if(GH_CLI)
    # Manual extraction for gh CLI downloads
    set(EXTRACT_DIR "${CURRENT_BUILDTREES_DIR}/src")
    file(MAKE_DIRECTORY "${EXTRACT_DIR}")
    
    if(ARCHIVE_SUFFIX STREQUAL ".tar.gz")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${ARCHIVE}"
            WORKING_DIRECTORY "${EXTRACT_DIR}"
            RESULT_VARIABLE EXTRACT_RESULT
        )
    elseif(ARCHIVE_SUFFIX STREQUAL ".zip")
        find_program(UNZIP unzip)
        if(UNZIP)
            execute_process(
                COMMAND ${UNZIP} -q "${ARCHIVE}" -d "${EXTRACT_DIR}"
                RESULT_VARIABLE EXTRACT_RESULT
            )
        else()
            message(FATAL_ERROR "unzip not found. Cannot extract ${ARCHIVE_NAME}")
        endif()
    endif()
    
    if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract ${ARCHIVE_NAME}")
    endif()
else()
    # Use vcpkg_extract_source_archive for vcpkg_download_distfile downloads
    vcpkg_extract_source_archive(
        SOURCE_PATH "${CURRENT_BUILDTREES_DIR}/src"
        ARCHIVE "${ARCHIVE}"
    )
endif()

# Install to vcpkg package directory (standard location)
file(COPY "${CURRENT_BUILDTREES_DIR}/src/ezd2step-${VERSION}-${PLATFORM}/"
     DESTINATION "${CURRENT_PACKAGES_DIR}/tools/ezd2step"
     FILES_MATCHING PATTERN "*")

# Copy to EZDesign resources directory
# Try to detect EZDesign project root from VCPKG_ROOT
# vcpkg is in vendor/vcpkg, so project root is VCPKG_ROOT/../..
if(DEFINED VCPKG_ROOT)
    get_filename_component(PROJECT_ROOT "${VCPKG_ROOT}/../.." ABSOLUTE)
    if(EXISTS "${PROJECT_ROOT}/package.json" AND EXISTS "${PROJECT_ROOT}/resources")
        set(RESOURCES_DIR "${PROJECT_ROOT}/resources/ezd2step")
        file(MAKE_DIRECTORY "${RESOURCES_DIR}")
        file(COPY "${CURRENT_PACKAGES_DIR}/tools/ezd2step/"
             DESTINATION "${RESOURCES_DIR}"
             FILES_MATCHING PATTERN "*")
        message(STATUS "ezd2step bundle copied to: ${RESOURCES_DIR}")
    else()
        message(WARNING "Could not find EZDesign project root from VCPKG_ROOT. Bundle installed to: ${CURRENT_PACKAGES_DIR}/tools/ezd2step")
    endif()
else()
    message(WARNING "VCPKG_ROOT not defined. Bundle installed to: ${CURRENT_PACKAGES_DIR}/tools/ezd2step")
endif()

# Cleanup temp extraction directory
file(REMOVE_RECURSE "${CURRENT_BUILDTREES_DIR}/src")
