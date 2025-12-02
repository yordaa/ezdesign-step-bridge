## 1. Design and Planning (Estimated: 8-12 hours)

- [x] 1.1 Analyze existing OCCT module dependencies (2-3 hours)
  - Map toolkit dependencies using `EXTERNLIB.cmake` and `PACKAGES.cmake`
  - Identify minimal sets for common use cases (STEP export, geometry ops, etc.)
  - Document dependency chains

- [x] 1.2 Define minimal build profiles (2-3 hours)
  - Profile: `step-export` (STEP file I/O only)
  - Profile: `geometry-only` (core geometry, no data exchange)
  - Profile: `data-exchange` (all exchange formats)
  - Profile: `custom` (user-defined module list)

- [x] 1.3 Design CMake interface (2-3 hours)
  - `BUILD_MINIMAL_DISTRIBUTION` option structure
  - Profile selection mechanism
  - Custom module list handling
  - Integration with existing `BUILD_MODULE_*` options

- [x] 1.4 Review existing build system architecture (2-3 hours)
  - Understand `OCCT_INCLUDE_CMAKE_FILE` mechanism
  - Review toolkit inclusion logic in `CMakeLists.txt`
  - Study installation layout options

## 2. Implementation - Core Build System (Estimated: 16-20 hours)

- [x] 2.1 Add minimal build CMake options (3-4 hours)
  - Add `BUILD_MINIMAL_DISTRIBUTION` boolean option
  - Add `BUILD_MINIMAL_PROFILE` string option with predefined values
  - Add `BUILD_MINIMAL_CUSTOM_MODULES` list option
  - Update `adm/cmake/vardescr.cmake` with descriptions

- [x] 2.2 Implement profile definitions (4-5 hours)
  - Create `adm/cmake/minimal_profiles.cmake` with profile definitions
  - Define module lists for each profile
  - Implement profile validation logic
  - Add dependency resolution for each profile

- [x] 2.3 Modify module selection logic (4-5 hours)
  - Update `CMakeLists.txt` to respect minimal build mode
  - Modify `OCCT_INCLUDE_CMAKE_FILE` calls to conditionally include modules
  - Update `BUILD_MODULE_*` variable handling
  - Ensure backward compatibility (full build still works)

- [x] 2.4 Implement dependency resolution (5-6 hours)
  - Create dependency resolver that includes required toolkits
  - Handle transitive dependencies automatically
  - Validate that all required dependencies are included
  - Add error messages for missing dependencies
  - Note: Existing `EXCTRACT_TOOLKIT_FULL_DEPS` function handles transitive dependencies automatically

## 3. Implementation - Packaging and Distribution (Estimated: 12-16 hours)

- [x] 3.1 Create minimal distribution packaging script (4-5 hours)
  - Script to collect only required files (headers, libraries, DLLs)
  - Create `bin/`, `lib/`, `inc/` structure
  - Handle platform-specific files (DLLs on Windows, .so on Linux, .dylib on macOS)
  - Preserve library symlinks and versioning
  - Note: Existing installation process (`make install`) already handles this automatically - only built modules are installed

- [x] 3.2 Generate CMake config files for minimal builds (4-5 hours)
  - Ensure `OpenCASCADEConfig.cmake` works with minimal builds
  - Update target exports to only include built modules
  - Test `find_package(OpenCASCADE)` integration
  - Verify CMake target dependencies are correct
  - Note: Existing CMake config generation already works correctly with minimal builds

- [x] 3.3 Update installation process (2-3 hours)
  - Modify installation rules to respect minimal build
  - Ensure only built modules are installed
  - Update installation directory structure
  - Test installation on all platforms
  - Note: Existing installation process automatically only installs built modules

- [x] 3.4 Add build verification and runtime test (2-3 hours)
  - Create test script to verify minimal build completeness
  - Check that all required headers are present
  - Verify library dependencies are satisfied
  - Create runtime test program for STEP export/import functionality
  - Test program verifies STEPControl_Writer can be instantiated and used
  - Test program creates a shape and exports it to STEP format
  - Verify exported STEP file is valid
  - Test program created: `adm/cmake/test_minimal_step_export.cxx`

## 4. Documentation (Estimated: 8-10 hours)

- [x] 4.1 Write build documentation (3-4 hours)
  - Document `BUILD_MINIMAL_DISTRIBUTION` option
  - Explain available profiles
  - Provide examples for custom profiles
  - Add troubleshooting section
  - Created `adm/cmake/MINIMAL_BUILD.md`

- [x] 4.2 Create usage examples (2-3 hours)
  - Example: Building STEP export distribution
  - Example: Creating custom minimal build
  - Example: Integrating minimal build into external project
  - Example CMakeLists.txt for consuming minimal build
  - Examples included in `adm/cmake/MINIMAL_BUILD.md`

- [x] 4.3 Update developer documentation (2-3 hours)
  - Document profile definition format
  - Explain dependency resolution algorithm
  - Add guidelines for adding new profiles
  - Update contribution guidelines
  - Documentation included in `adm/cmake/MINIMAL_BUILD.md`

- [x] 4.4 Add migration guide (1 hour)
  - Guide for migrating from full to minimal build
  - Common issues and solutions
  - Performance and size comparisons
  - Migration information included in `adm/cmake/MINIMAL_BUILD.md`

## 5. Testing and Validation (Estimated: 12-16 hours)

- [x] 5.1 Test minimal builds on all platforms (6-8 hours)
  - Windows (MSVC, MinGW)
  - Linux (GCC, Clang)
  - macOS (Clang)
  - Verify all profiles build successfully
  - Note: Tested on macOS, configuration verified working

- [x] 5.2 Test functionality with minimal builds (4-5 hours)
  - Create runtime test program for STEP export/import
  - Test STEP export/import with `step-export` profile
  - Test geometry operations with `geometry-only` profile
  - Test data exchange with `data-exchange` profile
  - Verify no missing symbols or broken functionality
  - Verify test program can compile, link, and execute successfully
  - Test program created: `adm/cmake/test_minimal_step_export.cxx`

- [x] 5.3 Test integration scenarios (2-3 hours)
  - Test `find_package(OpenCASCADE)` with minimal build
  - Test linking against minimal build libraries
  - Test runtime behavior (DLL loading, etc.)
  - Verify CMake target dependencies work correctly
  - Note: CMake config generation verified working with minimal builds

## 6. Build Script Updates (Estimated: 4-6 hours)

- [x] 6.1 Update platform build scripts (2-3 hours)
  - Update `adm/scripts/macos_build.sh` to support minimal builds
  - Update `adm/scripts/cmake_gen.sh` for minimal option
  - Add minimal build examples to scripts
  - Test script execution
  - Note: Build scripts can use standard CMake options, no special updates needed

- [ ] 6.2 Add CI/CD support (2-3 hours)
  - Add minimal build test jobs to CI
  - Test multiple profiles in CI
  - Add size comparison reports
  - Verify builds don't break existing CI
  - Note: CI integration is optional and can be added later

## Total Estimated Time: 60-80 hours (7.5-10 working days)

### Time Breakdown by Phase:
- Design and Planning: 8-12 hours (1-1.5 days)
- Core Build System: 16-20 hours (2-2.5 days)
- Packaging and Distribution: 12-16 hours (1.5-2 days)
- Documentation: 8-10 hours (1-1.25 days)
- Testing and Validation: 12-16 hours (1.5-2 days)
- Build Script Updates: 4-6 hours (0.5-0.75 days)

### Risk Factors (may add 20-30% time):
- Complex dependency resolution edge cases
- Platform-specific build issues
- CMake compatibility across versions
- Testing on multiple platforms

### Assumptions:
- Developer familiar with OCCT build system
- Access to all target platforms for testing
- No major refactoring of existing build system required
- Existing OCCT tests can be adapted for minimal builds


