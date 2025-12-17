# Portfile for ezd2step - EZDesign to STEP converter
# Downloads pre-built bundle from GitHub Releases

# Detect platform using vcpkg variables (not APPLE/WIN32 which don't work in script mode)
if(VCPKG_TARGET_IS_OSX AND VCPKG_TARGET_ARCHITECTURE STREQUAL "arm64")
    set(PLATFORM "macos-arm64")
    set(ARCHIVE_NAME "ezd2step-${VERSION}-macos-arm64.tar.gz")
    set(ARCHIVE_SHA256 "ed1f9b706ff3df5820bfeab04852d15a4384c3e565f0bd8b806f9267c52140e8")
elseif(VCPKG_TARGET_IS_WINDOWS AND VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
    set(PLATFORM "windows-x64")
    set(ARCHIVE_NAME "ezd2step-${VERSION}-windows-x64.zip")
    set(ARCHIVE_SHA256 "bff27413d49433ac820415411f970ff9b6142cfc584d58abe2441d8ab2b987ba")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

find_program(CURL curl REQUIRED)

set(ARCHIVE "${DOWNLOADS}/${ARCHIVE_NAME}")

set(NEED_DOWNLOAD TRUE)
if(EXISTS "${ARCHIVE}")
    file(SHA256 "${ARCHIVE}" EXISTING_SHA256)
    if(EXISTING_SHA256 STREQUAL ARCHIVE_SHA256)
        set(NEED_DOWNLOAD FALSE)
    endif()
endif()

if(NEED_DOWNLOAD)
    set(AUTH_TOKEN "$ENV{PAT_TOKEN}")
    if("${AUTH_TOKEN}" STREQUAL "")
        message(FATAL_ERROR "PAT_TOKEN is required and must not be empty")
    endif()
    
    set(RELEASE_INFO "${CURRENT_BUILDTREES_DIR}/release_info.json")
    execute_process(
        COMMAND ${CURL} -fsSL
            -H "Accept: application/vnd.github+json"
            -H "Authorization: token ${AUTH_TOKEN}"
            -H "X-GitHub-Api-Version: 2022-11-28"
            -o "${RELEASE_INFO}"
            "https://api.github.com/repos/yordaa/ezdesign-step-bridge/releases/tags/v${VERSION}"
        RESULT_VARIABLE CURL_RESULT ERROR_VARIABLE CURL_ERROR
    )
    if(CURL_RESULT)
        message(FATAL_ERROR "Failed to fetch release: ${CURL_ERROR}")
    endif()

    file(READ "${RELEASE_INFO}" RELEASE_JSON)

    string(JSON ASSETS_LEN LENGTH "${RELEASE_JSON}" assets)
    message(STATUS "GitHub release assets count: ${ASSETS_LEN}")

    set(ASSET_ID "")
    math(EXPR LAST_INDEX "${ASSETS_LEN} - 1")
    foreach(i RANGE 0 ${LAST_INDEX})
        string(JSON ASSET_NAME GET "${RELEASE_JSON}" assets ${i} name)
        if(ASSET_NAME STREQUAL "${ARCHIVE_NAME}")
            string(JSON ASSET_ID GET "${RELEASE_JSON}" assets ${i} id)
            break()
        endif()
    endforeach()

    if(ASSET_ID STREQUAL "")
        message(FATAL_ERROR "Asset ${ARCHIVE_NAME} not found")
    endif()

    message(STATUS "Using GitHub release asset id: ${ASSET_ID}")
    
    execute_process(
        COMMAND ${CURL} -fsSL
            -H "Accept: application/octet-stream"
            -H "Authorization: token ${AUTH_TOKEN}"
            -H "X-GitHub-Api-Version: 2022-11-28"
            -o "${ARCHIVE}"
            "https://api.github.com/repos/yordaa/ezdesign-step-bridge/releases/assets/${ASSET_ID}"
        RESULT_VARIABLE CURL_RESULT ERROR_VARIABLE CURL_ERROR
    )
    if(CURL_RESULT)
        message(FATAL_ERROR "Download failed: ${CURL_ERROR}")
    endif()
    
    file(SHA256 "${ARCHIVE}" DOWNLOADED_SHA256)
    if(NOT DOWNLOADED_SHA256 STREQUAL ARCHIVE_SHA256)
        message(FATAL_ERROR "SHA256 mismatch")
    endif()
    
    file(REMOVE "${RELEASE_INFO}")
endif()

set(EXTRACT_DIR "${CURRENT_BUILDTREES_DIR}/src")

file(ARCHIVE_EXTRACT
    INPUT "${ARCHIVE}"
    DESTINATION "${EXTRACT_DIR}"
)

file(COPY "${CURRENT_BUILDTREES_DIR}/src/ezd2step-${VERSION}-${PLATFORM}/"
     DESTINATION "${CURRENT_PACKAGES_DIR}/tools/ezd2step" FILES_MATCHING PATTERN "*")

file(REMOVE_RECURSE "${EXTRACT_DIR}")
