<!-- OPENSPEC:START -->
# OpenSpec Instructions

These instructions are for AI assistants working in this project.

Always open `@/openspec/AGENTS.md` when the request:
- Mentions planning or proposals (words like proposal, spec, change, plan)
- Introduces new capabilities, breaking changes, architecture shifts, or big performance/security work
- Sounds ambiguous and you need the authoritative spec before coding

Use `@/openspec/AGENTS.md` to learn:
- How to create and apply change proposals
- Spec format and conventions
- Project structure and guidelines

Keep this managed block so 'openspec update' can refresh the instructions.

<!-- OPENSPEC:END -->

## Building

For build instructions, prerequisites, and build configuration:

**Always refer to `@/README.md`** - see the "Building" section for:
- Prerequisites (CMake, C++ compiler, vcpkg)
- vcpkg setup instructions (clone and bootstrap)
- Build commands using vcpkg toolchain (`cmake -B build -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release`)
- Executable locations for different platforms
- Test suite build instructions

**Note**: The build commands in README.md build in **Release** mode explicitly. The ezd2step CMakeLists.txt automatically disables unnecessary modules (USE_TK=OFF, BUILD_MODULE_Draw=OFF, USE_FREETYPE=OFF) since it's a command-line tool. vcpkg manifest mode automatically handles the `nlohmann-json` dependency via the root manifest at `adm/vcpkg/vcpkg.json`.

## vcpkg Port/Registry Updates

When updating vcpkg ports or the registry (e.g., updating ezd2step version, fixing portfile issues, updating baseline):

**Always refer to `@/VCPKG_PORT_UPDATE_PROCESS.md`** for the complete step-by-step process.

Key points to remember:
- GitHub release with binaries must exist **before** updating the portfile
- Always regenerate version file using `vcpkg x-add-version` after committing port changes
- Verify git-tree hash matches: `git rev-parse HEAD:ports/ezd2step`
- Update baseline in `versions/baseline.json` to match new version
- Follow the workflow order: Build → Release → Port Update → Registry Update

See `VCPKG_PORT_UPDATE_PROCESS.md` for detailed instructions, common pitfalls, and verification checklists.