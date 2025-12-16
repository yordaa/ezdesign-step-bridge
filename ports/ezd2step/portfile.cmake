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
vcpkg_download_distfile(
    ARCHIVE
    URLS "https://github.com/yordaa/ezdesign-step-bridge/releases/download/v${VERSION}/${ARCHIVE_NAME}"
    FILENAME "${ARCHIVE_NAME}"
    SHA512 "${ARCHIVE_SHA512}"
)

# Extract archive
vcpkg_extract_source_archive(
    ARCHIVE "${ARCHIVE}"
    SOURCE_PATH "${CURRENT_BUILDTREES_DIR}/src"
)

# Install to vcpkg package directory (standard location)
file(COPY "${CURRENT_BUILDTREES_DIR}/src/ezd2step-${VERSION}-${PLATFORM}/"
     DESTINATION "${CURRENT_PACKAGES_DIR}/tools/ezd2step"
     FILES_MATCHING PATTERN "*")

# Optional: Copy to EZDesign resources directory if variable is set
# This allows EZDesign to use the bundle directly from resources/
if(DEFINED EZDESIGN_RESOURCES_DIR)
    file(MAKE_DIRECTORY "${EZDESIGN_RESOURCES_DIR}/ezd2step")
    file(COPY "${CURRENT_PACKAGES_DIR}/tools/ezd2step/"
         DESTINATION "${EZDESIGN_RESOURCES_DIR}/ezd2step"
         FILES_MATCHING PATTERN "*")
    message(STATUS "ezd2step bundle copied to: ${EZDESIGN_RESOURCES_DIR}/ezd2step")
endif()

# Cleanup temp extraction directory
file(REMOVE_RECURSE "${CURRENT_BUILDTREES_DIR}/src")
