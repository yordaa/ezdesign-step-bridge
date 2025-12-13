#!/bin/bash
#=======================================================================
# Script: Create SHA256 checksums for ezd2step bundles
# Purpose: Generate SHA256SUMS file for distribution artifacts
#=======================================================================

set -e

# Get absolute path to bundles directory
if [ -n "${1}" ]; then
  BUNDLES_DIR="$(cd "${1}" && pwd)"
else
  # Default: assume script is run from project root
  SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
  PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
  BUNDLES_DIR="${PROJECT_ROOT}/build/bundles"
fi

CHECKSUMS_FILE="${BUNDLES_DIR}/SHA256SUMS"

if [ ! -d "${BUNDLES_DIR}" ]; then
  echo "Error: Bundles directory not found: ${BUNDLES_DIR}"
  exit 1
fi

echo "Generating SHA256 checksums for bundles in ${BUNDLES_DIR}..."

# Remove existing checksums file
rm -f "${CHECKSUMS_FILE}"

# Change to bundles directory to compute relative paths
cd "${BUNDLES_DIR}"

# Generate checksums for all archives (process each file only once)
# Use find to avoid glob pattern issues
find . -maxdepth 1 -type f \( -name "*.tar.gz" -o -name "*.zip" \) | while read -r archive; do
  archive_name=$(basename "${archive}")
  # Skip if already in checksums file
  if ! grep -q "^[0-9a-f]*  ${archive_name}" "${CHECKSUMS_FILE}" 2>/dev/null; then
    echo "Computing checksum for ${archive_name}..."
    if command -v sha256sum >/dev/null 2>&1; then
      sha256sum "${archive_name}" >> "${CHECKSUMS_FILE}"
    elif command -v shasum >/dev/null 2>&1; then
      shasum -a 256 "${archive_name}" >> "${CHECKSUMS_FILE}"
    else
      echo "Error: Neither sha256sum nor shasum found"
      exit 1
    fi
  fi
done

echo "Checksums written to ${CHECKSUMS_FILE}"
cat "${CHECKSUMS_FILE}"

