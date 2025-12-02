## Context

OCCT is a large library (500MB+ full installation) with many modules. Many applications only need a subset (e.g., STEP file export), but currently must include everything. A colleague created a manual stripped-down version (~123MB) by selectively copying files, but this approach:
- Requires manual maintenance
- Loses CMake integration
- Is platform-specific (Windows only in their case)
- Has no dependency validation

This proposal adds official support for building minimal OCCT distributions through CMake configuration.

## Goals / Non-Goals

### Goals
- Enable building OCCT with only required modules
- Provide predefined profiles for common use cases
- Maintain full CMake integration (`find_package` support)
- Support all OCCT platforms (Windows, Linux, macOS)
- Automatically resolve and include dependencies
- Reduce distribution size by 60-70% for typical use cases

### Non-Goals
- Breaking existing full builds (must remain default)
- Supporting every possible module combination (focus on common profiles)
- Runtime module loading (static build-time selection only)
- Automatic profile detection (explicit selection required)

## Decisions

### Decision 1: Profile-Based Approach
**What**: Use predefined profiles (`step-export`, `geometry-only`, etc.) plus custom module lists.

**Why**: 
- Easier to use than manual module selection
- Validated combinations reduce errors
- Common use cases are well-defined

**Alternatives considered**:
- Manual module selection only: Too complex for users
- Automatic dependency detection: Too risky, may miss required modules

### Decision 2: CMake Option Integration
**What**: Add `BUILD_MINIMAL_DISTRIBUTION` option that works alongside existing `BUILD_MODULE_*` options.

**Why**:
- Maintains backward compatibility
- Allows gradual migration
- Leverages existing build infrastructure

**Alternatives considered**:
- Separate build system: Too much duplication
- Replace existing system: Too risky, breaks existing workflows

### Decision 3: Dependency Resolution Algorithm
**What**: Recursively resolve dependencies using `EXTERNLIB.cmake` and `PACKAGES.cmake` files, include all transitive dependencies.

**Why**:
- Ensures builds are complete
- Prevents missing symbol errors
- Uses existing dependency information

**Alternatives considered**:
- Manual dependency lists: Too error-prone
- Runtime dependency checking: Too late, build would fail

### Decision 4: Preserve CMake Config Files
**What**: Generate full `OpenCASCADEConfig.cmake` even for minimal builds, but only export built targets.

**Why**:
- Enables `find_package(OpenCASCADE)` to work
- Maintains integration with external projects
- Only exports what's actually built

**Alternatives considered**:
- Skip CMake config: Breaks integration
- Generate minimal config: Too complex, loses compatibility

### Decision 5: Installation Layout
**What**: Use same installation layout as full builds (`bin/`, `lib/`, `include/opencascade/`), just with fewer files.

**Why**:
- Consistent with existing structure
- Easier for users to understand
- Compatible with existing integration code

**Alternatives considered**:
- Custom layout: Unnecessary complexity
- Flat structure: Loses organization benefits

## Risks / Trade-offs

### Risk 1: Missing Dependencies
**Risk**: Dependency resolution misses required modules, causing runtime errors.

**Mitigation**: 
- Comprehensive testing of each profile
- Clear error messages for missing dependencies
- Documentation of profile contents

### Risk 2: Build System Complexity
**Risk**: Adding minimal build logic complicates CMake files.

**Mitigation**:
- Isolate minimal build logic in separate CMake files
- Maintain clear separation from full build logic
- Extensive comments and documentation

### Risk 3: Maintenance Burden
**Risk**: Profiles become outdated as OCCT evolves.

**Mitigation**:
- Document profile update process
- Add CI tests for profiles
- Version profiles with OCCT releases

### Risk 4: Platform-Specific Issues
**Risk**: Minimal builds work differently on different platforms.

**Mitigation**:
- Test on all platforms
- Platform-specific documentation
- Clear error messages for platform issues

### Trade-offs
- **Size vs. Completeness**: Smaller builds but must explicitly select modules
- **Simplicity vs. Flexibility**: Profiles are simpler but less flexible than manual selection
- **Maintenance vs. Features**: More profiles = more maintenance, but better UX

## Migration Plan

### Phase 1: Implementation (Weeks 1-2)
- Implement core build system changes
- Add basic profiles
- Test on primary platform

### Phase 2: Testing (Week 3)
- Test all profiles on all platforms
- Validate functionality
- Fix issues

### Phase 3: Documentation (Week 4)
- Write user documentation
- Create examples
- Update developer guides

### Phase 4: Release (Week 5)
- Merge to main branch
- Announce feature
- Monitor for issues

### Rollback Plan
- Feature is opt-in (`BUILD_MINIMAL_DISTRIBUTION` defaults to OFF)
- Full builds remain unchanged
- Can disable feature via CMake option if issues arise

## Open Questions

1. **Profile Naming**: Are `step-export`, `geometry-only`, `data-exchange` the right names? Should we use more descriptive names?
2. **Custom Profile Format**: What format should users use to define custom profiles? CMake list? JSON? Separate CMake file?
3. **Version Compatibility**: Should profiles be versioned with OCCT releases? How do we handle profile changes?
4. **Size Targets**: What are acceptable size targets? 50% reduction? 70%?
5. **Testing Strategy**: Should we add automated tests for minimal builds? How comprehensive?


