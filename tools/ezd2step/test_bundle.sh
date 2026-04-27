#!/bin/bash
#=======================================================================
# Script: Smoke tests for ezd2step bundle validation
# Purpose: Validate bundle functionality and library loading
#=======================================================================

# Don't use set -e, we want to handle errors manually
set +e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

# Function to print test result
print_test() {
    local test_name="$1"
    local status="$2"
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}✓${NC} $test_name"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗${NC} $test_name"
        ((TESTS_FAILED++))
    fi
}

# Function to check if command exists
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo -e "${RED}Error: $1 not found${NC}"
        exit 1
    fi
}

echo "=========================================="
echo "ezd2step Bundle Smoke Tests"
echo "=========================================="
echo ""

# Parse arguments
BUNDLE_ARCHIVE="${1:-build/bundles/ezd2step-1.0.0-macos-arm64.tar.gz}"
TEST_DIR="${2:-/tmp/ezd2step_bundle_test}"
SAMPLE_EZD="${3:-}"

if [ ! -f "$BUNDLE_ARCHIVE" ]; then
    echo -e "${RED}Error: Bundle archive not found: $BUNDLE_ARCHIVE${NC}"
    echo "Usage: $0 [bundle_archive] [test_directory] [sample_ezd]"
    exit 1
fi

BUNDLE_ARCHIVE="$(cd "$(dirname "$BUNDLE_ARCHIVE")" && pwd)/$(basename "$BUNDLE_ARCHIVE")"
if [ -n "$SAMPLE_EZD" ] && [ -f "$SAMPLE_EZD" ]; then
    SAMPLE_EZD="$(cd "$(dirname "$SAMPLE_EZD")" && pwd)/$(basename "$SAMPLE_EZD")"
fi

case "$BUNDLE_ARCHIVE" in
    *.tar.gz)
        check_command "tar"
        EXTRACT_COMMAND=(tar -xzf "$BUNDLE_ARCHIVE")
        ;;
    *.zip)
        check_command "unzip"
        EXTRACT_COMMAND=(unzip -q "$BUNDLE_ARCHIVE")
        ;;
    *)
        echo -e "${RED}Error: Unsupported bundle archive format: $BUNDLE_ARCHIVE${NC}"
        exit 1
        ;;
esac

echo "Bundle archive: $BUNDLE_ARCHIVE"
echo "Test directory: $TEST_DIR"
if [ -n "$SAMPLE_EZD" ]; then
    echo "Sample EZD: $SAMPLE_EZD"
fi
echo ""

# Clean up test directory
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"

# Extract bundle
echo "Extracting bundle..."
cd "$TEST_DIR"
"${EXTRACT_COMMAND[@]}" || {
    echo -e "${RED}Error: Failed to extract bundle${NC}"
    exit 1
}

BUNDLE_NAME=$(basename "$BUNDLE_ARCHIVE" .tar.gz)
BUNDLE_NAME=$(basename "$BUNDLE_NAME" .zip)
BUNDLE_DIR="$TEST_DIR/$BUNDLE_NAME"

if [ ! -d "$BUNDLE_DIR" ]; then
    echo -e "${RED}Error: Bundle directory not found after extraction${NC}"
    exit 1
fi

cd "$BUNDLE_DIR"

echo ""
echo "=========================================="
echo "Test 1: Executable exists and is executable"
echo "=========================================="
if [ -x "./ezd2step" ]; then
    print_test "Executable exists and is executable" "PASS"
else
    print_test "Executable exists and is executable" "FAIL"
    exit 1
fi

echo ""
echo "=========================================="
echo "Test 2: --version flag"
echo "=========================================="
VERSION_OUTPUT=$(./ezd2step --version 2>&1)
VERSION_EXIT=$?
if [ $VERSION_EXIT -eq 0 ] && echo "$VERSION_OUTPUT" | grep -q "ezd2step version"; then
    print_test "--version flag works" "PASS"
    echo "  Version output: $VERSION_OUTPUT"
else
    print_test "--version flag works" "FAIL"
    echo "  Exit code: $VERSION_EXIT"
    echo "  Output: $VERSION_OUTPUT"
fi

echo ""
echo "=========================================="
echo "Test 3: --help flag"
echo "=========================================="
HELP_OUTPUT=$(./ezd2step --help 2>&1)
HELP_EXIT=$?
if [ $HELP_EXIT -eq 0 ] && echo "$HELP_OUTPUT" | grep -q "Usage:"; then
    print_test "--help flag works" "PASS"
else
    print_test "--help flag works" "FAIL"
    echo "  Exit code: $HELP_EXIT"
fi

echo ""
echo "=========================================="
echo "Test 4: Invalid arguments (exit code 1)"
echo "=========================================="
if ./ezd2step > /dev/null 2>&1; then
    print_test "Invalid arguments return exit code 1" "FAIL"
else
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 1 ]; then
        print_test "Invalid arguments return exit code 1" "PASS"
    else
        print_test "Invalid arguments return exit code 1 (got $EXIT_CODE)" "FAIL"
    fi
fi

echo ""
echo "=========================================="
echo "Test 5: Missing input file (exit code 2)"
echo "=========================================="
if ./ezd2step nonexistent.ezd output.step > /dev/null 2>&1; then
    print_test "Missing input file returns exit code 2" "FAIL"
else
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 2 ]; then
        print_test "Missing input file returns exit code 2" "PASS"
    else
        print_test "Missing input file returns exit code 2 (got $EXIT_CODE)" "FAIL"
    fi
fi

echo ""
echo "=========================================="
echo "Test 6: Library loading (check dependencies)"
echo "=========================================="
if command -v otool > /dev/null 2>&1; then
    # macOS: Check library dependencies
    DEPS=$(otool -L ./ezd2step 2>/dev/null | grep -c "@rpath/libTK" || true)
    if [ "$DEPS" -gt 0 ]; then
        print_test "Library dependencies found ($DEPS OCCT libraries)" "PASS"
    else
        print_test "Library dependencies found" "FAIL"
    fi
elif command -v ldd > /dev/null 2>&1; then
    # Linux: Check library dependencies
    DEPS=$(ldd ./ezd2step 2>/dev/null | grep -c "libTK" || true)
    if [ "$DEPS" -gt 0 ]; then
        print_test "Library dependencies found ($DEPS OCCT libraries)" "PASS"
    else
        print_test "Library dependencies found" "FAIL"
    fi
else
    print_test "Library dependencies check (tool not available)" "SKIP"
fi

echo ""
echo "=========================================="
echo "Test 7: Required OCCT libraries present"
echo "=========================================="
REQUIRED_LIBS=("libTKDESTEP" "libTKXSBase" "libTKDE" "libTKBRep" "libTKernel")
MISSING_LIBS=()
for lib in "${REQUIRED_LIBS[@]}"; do
    # Check for versioned and unversioned library names
    if ls ${lib}*.dylib ${lib}*.so ${lib}*.dll 2>/dev/null | grep -q .; then
        continue
    else
        MISSING_LIBS+=("$lib")
    fi
done

if [ ${#MISSING_LIBS[@]} -eq 0 ]; then
    print_test "Required OCCT libraries present" "PASS"
else
    print_test "Required OCCT libraries present (missing: ${MISSING_LIBS[*]})" "FAIL"
fi

echo ""
echo "=========================================="
echo "Test 8: Convert sample .ezd file"
echo "=========================================="
if [ -n "$SAMPLE_EZD" ]; then
    if [ ! -f "$SAMPLE_EZD" ]; then
        print_test "Sample EZD exists" "FAIL"
        echo "  Missing sample: $SAMPLE_EZD"
    else
        cp "$SAMPLE_EZD" ./sample.ezd
        ./ezd2step sample.ezd sample.step > conversion.log 2>&1
        CONVERSION_EXIT=$?
        if [ $CONVERSION_EXIT -eq 0 ] && [ -s sample.step ]; then
            print_test "Sample conversion creates STEP output" "PASS"
        else
            print_test "Sample conversion creates STEP output" "FAIL"
            echo "  Exit code: $CONVERSION_EXIT"
            echo "  Conversion output:"
            cat conversion.log
        fi
    fi
else
    print_test "Sample conversion skipped (no sample provided)" "PASS"
fi

echo ""
echo "=========================================="
echo "Test 9: README.txt exists"
echo "=========================================="
if [ -f "README.txt" ]; then
    print_test "README.txt exists" "PASS"
else
    print_test "README.txt exists" "FAIL"
fi

echo ""
echo "=========================================="
echo "Test 10: LICENSE.txt exists"
echo "=========================================="
if [ -f "LICENSE.txt" ]; then
    print_test "LICENSE.txt exists" "PASS"
else
    print_test "LICENSE.txt exists" "FAIL"
fi

echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
if [ $TESTS_FAILED -gt 0 ]; then
    echo -e "${RED}Failed: $TESTS_FAILED${NC}"
    exit 1
else
    echo -e "${GREEN}Failed: $TESTS_FAILED${NC}"
    echo ""
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi

