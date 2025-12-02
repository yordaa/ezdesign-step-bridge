# Scope and Timeline: Minimal OCCT Build Configuration

## Executive Summary

This proposal adds official support for building minimal OCCT distributions through CMake configuration, enabling developers to create lightweight OCCT installations (150-200MB vs 500MB+ full builds) with only required modules.

**Total Estimated Time**: 60-80 hours (7.5-10 working days)

## Scope Breakdown

### Phase 1: Design and Planning (8-12 hours)
**Duration**: 1-1.5 days

- Analyze existing OCCT module dependencies
- Define minimal build profiles (step-export, geometry-only, data-exchange, custom)
- Design CMake interface for minimal builds
- Review existing build system architecture

**Deliverables**:
- Dependency map document
- Profile definitions
- CMake interface design document

### Phase 2: Core Build System Implementation (16-20 hours)
**Duration**: 2-2.5 days

- Add CMake options for minimal builds
- Implement profile definitions
- Modify module selection logic
- Implement automatic dependency resolution

**Deliverables**:
- Modified `CMakeLists.txt` files
- New `adm/cmake/minimal_profiles.cmake`
- Updated build system with minimal build support

### Phase 3: Packaging and Distribution (12-16 hours)
**Duration**: 1.5-2 days

- Create minimal distribution packaging script
- Generate CMake config files for minimal builds
- Update installation process
- Add build verification

**Deliverables**:
- Packaging script
- Updated CMake config generation
- Installation verification tools

### Phase 4: Documentation (8-10 hours)
**Duration**: 1-1.25 days

- Write build documentation
- Create usage examples
- Update developer documentation
- Add migration guide

**Deliverables**:
- User documentation
- Example projects
- Developer guide updates

### Phase 5: Testing and Validation (12-16 hours)
**Duration**: 1.5-2 days

- Test minimal builds on all platforms (Windows, Linux, macOS)
- Test functionality with minimal builds
- Test integration scenarios

**Deliverables**:
- Test reports
- Platform-specific validation
- Integration test results

### Phase 6: Build Script Updates (4-6 hours)
**Duration**: 0.5-0.75 days

- Update platform build scripts
- Add CI/CD support

**Deliverables**:
- Updated build scripts
- CI configuration updates

## Timeline Visualization

```
Week 1: Design + Core Implementation
├── Days 1-2: Design and Planning (8-12h)
└── Days 3-5: Core Build System (16-20h)

Week 2: Packaging + Documentation
├── Days 1-2: Packaging and Distribution (12-16h)
└── Days 3-4: Documentation (8-10h)

Week 3: Testing + Polish
├── Days 1-3: Testing and Validation (12-16h)
└── Day 4-5: Build Script Updates (4-6h)
```

**Total Calendar Time**: 3 weeks (15 working days)
**Total Effort**: 60-80 hours (7.5-10 working days)

## Risk Factors

### High Impact Risks (+20-30% time)
1. **Complex Dependency Resolution**: Edge cases in dependency resolution may require additional debugging
2. **Platform-Specific Issues**: Different behavior on Windows/Linux/macOS may need platform-specific fixes
3. **CMake Compatibility**: Ensuring compatibility across CMake versions (3.10-3.31+) may require workarounds

### Medium Impact Risks (+10-15% time)
1. **Testing Coverage**: Comprehensive testing across all platforms and profiles may reveal additional issues
2. **Documentation Completeness**: Ensuring documentation covers all use cases may require iterations

### Low Impact Risks (+5% time)
1. **Build Script Updates**: Minor adjustments to existing scripts
2. **CI/CD Integration**: Standard CI configuration updates

## Success Criteria

### Functional Requirements
- ✅ Minimal builds work on all supported platforms
- ✅ All predefined profiles build successfully
- ✅ Custom module lists work correctly
- ✅ Dependency resolution includes all required modules
- ✅ CMake integration (`find_package`) works with minimal builds
- ✅ Installation produces correct directory structure

### Quality Requirements
- ✅ Distribution size reduced by 60-70% for typical profiles
- ✅ No missing symbols or broken functionality
- ✅ Backward compatibility maintained (full builds unchanged)
- ✅ Clear error messages for configuration issues
- ✅ Comprehensive documentation provided

### Performance Requirements
- ✅ Build time for minimal builds is faster than full builds
- ✅ No significant overhead in CMake configuration time
- ✅ Dependency resolution completes in reasonable time (<30 seconds)

## Resource Requirements

### Skills Required
- CMake expertise (advanced)
- OCCT build system knowledge
- C++ build system understanding
- Cross-platform development experience
- Technical writing (for documentation)

### Infrastructure Required
- Access to Windows, Linux, and macOS build environments
- CI/CD system for automated testing
- Version control system (Git)

### Dependencies
- No new external dependencies
- Existing OCCT dependencies (CMake, Tcl, etc.)

## Comparison with Alternative Approaches

### Option A: Manual Stripping (Current Colleague's Approach)
- **Time**: 2-4 hours (one-time)
- **Maintenance**: High (manual updates needed)
- **Platform Support**: Single platform
- **Integration**: Poor (no CMake support)
- **Reliability**: Medium (manual errors possible)

### Option B: This Proposal (Official Minimal Build Support)
- **Time**: 60-80 hours (one-time implementation)
- **Maintenance**: Low (automated, part of build system)
- **Platform Support**: All platforms
- **Integration**: Excellent (full CMake support)
- **Reliability**: High (automated, tested)

### Option C: Full OCCT Build (Status Quo)
- **Time**: 0 hours (already exists)
- **Maintenance**: Low (standard OCCT maintenance)
- **Platform Support**: All platforms
- **Integration**: Excellent
- **Reliability**: High
- **Size**: Large (500MB+)

## Recommendation

**Proceed with Option B (This Proposal)** because:
1. One-time investment (60-80 hours) provides long-term value
2. Automated solution reduces maintenance burden
3. Full platform and CMake support
4. Enables future optimizations and customizations
5. Aligns with OCCT's goal of being developer-friendly

The initial investment is justified by:
- Reduced distribution sizes (60-70% reduction)
- Better developer experience
- Official support for common use cases
- Foundation for future optimizations


