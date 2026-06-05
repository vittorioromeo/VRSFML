# CMake Codebase Review

## TL;DR

- The top-level `CMakeLists.txt` leans on `add_definitions()` for ~15 build-time toggles ([CMakeLists.txt:48-110](CMakeLists.txt#L48-L110)) and then re-issues the same macros inside `zancle_add_library` ([cmake/Macros.cmake:123-138](cmake/Macros.cmake#L123-L138)). Pick one mechanism (an `INTERFACE` "zancle-build-options" target) and delete the duplication; this alone removes ~80 lines and a class of subtle "did the define propagate?" bugs.
- The 5 per-multimedia-module `CMakeLists.txt` files repeat the same OpenGL/EGL/GLES branch verbatim ([Window:124-131](src/Zancle/Window/CMakeLists.txt#L124-L131), [GLUtils:71-80](src/Zancle/GLUtils/CMakeLists.txt#L71-L80), [ImGui:16-25](src/Zancle/ImGui/CMakeLists.txt#L16-L25)) plus the same macOS/iOS framework lists ([Window:148-153](src/Zancle/Window/CMakeLists.txt#L148-L153), [GLUtils:84-89](src/Zancle/GLUtils/CMakeLists.txt#L84-L89)). Factor into an `INTERFACE` GL backend target and Apple frameworks target.
- `zancle_add_library` is doing too much (350 lines, every concern in one macro). Split it into discrete helpers (`_create_target`, `_install_target`, `_setup_pdb`, `_setup_frameworks`) so each is independently understandable and the macro can compose them. The PDB block alone ([Macros.cmake:225-257](cmake/Macros.cmake#L225-L257)) is opaque.
- Test CMakeLists boilerplate is mostly identical one-liners except for Audio/Graphics resource bundling. Move the resource-staging logic into `zancle_add_test` (add a `RESOURCES` parameter) so the per-test files go back to being one line.
- Per-module `Export.hpp` files ([include/Zancle/Graphics/Export.hpp](include/Zancle/Graphics/Export.hpp), Audio, Window, ImGui, Network) are five identical 3-line stubs that should have been removed when `ZA_SYSTEM_API` was inlined into `Config.hpp`. Either delete them and inline the same pattern into `Config.hpp`, or generate them with `generate_export_header()` for free.

## 1. High-impact simplifications

### 1.1 Centralize build-toggle propagation -- effort: small

`CMakeLists.txt` declares booleans (`ZA_BUILD_GLUTILS`, `ZA_BUILD_WINDOW`, ..., `ZA_ENABLE_PCH`, `ZA_ENABLE_STACK_TRACES`, `ZA_ENABLE_LIFETIME_TRACKING`, `ZA_OPENGL_ES`) and immediately emits `add_definitions(-DZA_...)` for each ([CMakeLists.txt:88-110](CMakeLists.txt#L88-L110), [CMakeLists.txt:58-78](CMakeLists.txt#L58-L78), [CMakeLists.txt:189-191](CMakeLists.txt#L189-L191)). Then `zancle_add_library` re-emits four of the same defines as PUBLIC `target_compile_definitions` because (per the comment at [Macros.cmake:123](cmake/Macros.cmake#L123)) "`add_definitions` does not propagate to parent". That comment is the smoking gun: `add_definitions` is directory-scoped, so the original calls only reach TUs configured below that point, while the macro versions reach consumers of the library. Both fire for every Zancle TU.

Fix: create one `INTERFACE` target up front that owns the entire propagation surface:

```cmake
add_library(zancle-build-options INTERFACE)
target_compile_definitions(zancle-build-options INTERFACE
    $<$<BOOL:${ZA_BUILD_GLUTILS}>:ZA_BUILD_GLUTILS>
    $<$<BOOL:${ZA_BUILD_WINDOW}>:ZA_BUILD_WINDOW>
    ...
    $<$<BOOL:${ZA_ENABLE_PCH}>:ZA_ENABLE_PCH>
    $<$<BOOL:${ZA_OPENGL_ES}>:ZA_OPENGL_ES>)
target_compile_features(zancle-build-options INTERFACE cxx_std_23)
```

Then `zancle_add_library` does `target_link_libraries(${target} PUBLIC zancle-build-options)` and the 20 lines of `add_definitions` plus the 16 lines inside the macro disappear. Side benefit: examples and tests link the same INTERFACE target instead of relying on inherited directory state.

### 1.2 Factor the GL backend choice into one helper -- effort: small

Identical block, repeated three times:

- [src/Zancle/GLUtils/CMakeLists.txt:71-80](src/Zancle/GLUtils/CMakeLists.txt#L71-L80)
- [src/Zancle/Window/CMakeLists.txt:122-131](src/Zancle/Window/CMakeLists.txt#L122-L131)
- [src/Zancle/ImGui/CMakeLists.txt:16-25](src/Zancle/ImGui/CMakeLists.txt#L16-L25)

```cmake
if(ZA_OS_IOS)
    target_link_libraries(${T} PRIVATE "-framework OpenGLES")
elseif(ZA_OS_ANDROID OR ZA_OS_EMSCRIPTEN OR ZA_OPENGL_ES)
    find_package(EGL REQUIRED)
    find_package(GLES REQUIRED)
    target_link_libraries(${T} PRIVATE EGL::EGL GLES::GLES)
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(${T} PRIVATE OpenGL::GL)
endif()
```

Plus the Apple framework list ("Foundation", "AppKit", "IOKit", "Carbon" on macOS; "Foundation", "UIKit", "CoreGraphics", "QuartzCore", "CoreMotion" on iOS) is duplicated between Window and GLUtils ([Window:148-153](src/Zancle/Window/CMakeLists.txt#L148-L153), [GLUtils:84-89](src/Zancle/GLUtils/CMakeLists.txt#L84-L89)). Create one IMPORTED INTERFACE target:

```cmake
# in cmake/Macros.cmake or a new cmake/GLBackend.cmake
add_library(zancle::gl-backend INTERFACE IMPORTED GLOBAL)
add_library(zancle::apple-window-frameworks INTERFACE IMPORTED GLOBAL)
# ...populate once at top level...
```

Each module then does `target_link_libraries(zancle-graphics PRIVATE zancle::gl-backend)`. Removes ~30 lines and ensures consistency: today, ImGui doesn't add the Apple frameworks but GLUtils and Window do, which is suspicious and would be visible after factoring.

### 1.3 Extract third-party CPM wrappers -- effort: medium

`zancle_add_audio_dependencies` ([src/Zancle/Audio/CMakeLists.txt:19-142](src/Zancle/Audio/CMakeLists.txt#L19-L142)), `zancle_add_graphics_dependencies` ([src/Zancle/Graphics/CMakeLists.txt:33-88](src/Zancle/Graphics/CMakeLists.txt#L33-L88)) and `zancle_add_imgui_dependencies` ([src/Zancle/ImGui/CMakeLists.txt:28-91](src/Zancle/ImGui/CMakeLists.txt#L28-L91)) share a 12-line preamble (save `BUILD_SHARED_LIBS`, set `CMAKE_POLICY_DEFAULT_CMP0077 NEW`, override `BUILD_SHARED_LIBS=OFF`, after CPM call `zancle_set_stdlib`, `set_target_properties(... FOLDER Dependencies)`, set `POSITION_INDEPENDENT_CODE` when shared, set `UNITY_BUILD` flags, `target_compile_options -w`).

Define one helper:

```cmake
function(zancle_tame_thirdparty)
    set(opts UNITY_BUILD)
    set(single)
    set(multi TARGETS UNITY_SKIP_SOURCES)
    cmake_parse_arguments(ZTT "${opts}" "${single}" "${multi}" ${ARGN})
    foreach(t IN LISTS ZTT_TARGETS)
        if(NOT TARGET ${t})
            continue()
        endif()
        set_target_properties(${t} PROPERTIES FOLDER "Dependencies")
        if(BUILD_SHARED_LIBS)
            set_target_properties(${t} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif()
        if(ZTT_UNITY_BUILD)
            set_target_properties(${t} PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 256)
        else()
            set_target_properties(${t} PROPERTIES UNITY_BUILD OFF)
        endif()
        target_compile_options(${t} PRIVATE -w)
        zancle_set_stdlib(${t})
        zancle_apply_emscripten_options(${t})
    endforeach()
    foreach(s IN LISTS ZTT_UNITY_SKIP_SOURCES)
        set_source_files_properties(${s} PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)
    endforeach()
endfunction()
```

The Audio block's policy-default wrangling and the IIFE pattern (`function() ... endfunction(); zancle_add_audio_dependencies()`) is itself a smell -- it's just emulating CMake's `block()` (introduced in 3.25, supported under your 3.28 minimum). Network already uses `block()` correctly at [src/Zancle/Network/CMakeLists.txt:29-52](src/Zancle/Network/CMakeLists.txt#L29-L52); Audio/Graphics/ImGui should too.

### 1.4 Per-module `Export.hpp` files are vestigial -- effort: small

`include/Zancle/{Graphics,Audio,Window,ImGui,Network}/Export.hpp` are each 3-line files emitting `#define ZA_X_API ZA_API_EXPORT` based on a `ZA_X_EXPORTS` macro that CMake sets via `DEFINE_SYMBOL` ([Macros.cmake:162-164](cmake/Macros.cmake#L162-L164)). Meanwhile `Config.hpp` ([include/Zancle/Config.hpp:147-150](include/Zancle/Config.hpp#L147-L150)) now defines `ZA_SYSTEM_API` directly via the same pattern. Two ways to clean this up:

1. Inline all five `ZA_*_API` definitions into `Config.hpp` (consistent with what was done for System) and delete the per-module `Export.hpp` stubs. This is the simpler fork-friendly path.
2. Replace the hand-rolled scaffolding with `generate_export_header(${target} BASE_NAME ZA_${MODULE} EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/include/Zancle/${Module}/Export.hpp)`. Adds a build-dir include path but removes both the headers and the `DEFINE_SYMBOL` boilerplate.

Either way, also nuke the awful `string(REGEX REPLACE "zancle-([a-z])([a-z]*)" "\\1" RC_MODULE_NAME_HEAD ...)` capitalization hack at [Macros.cmake:178-181](cmake/Macros.cmake#L178-L181) -- pass the cased `module` name (which the caller already supplies) directly into the .rc.in template.

### 1.5 Per-test boilerplate -- effort: small

[test/Audio/CMakeLists.txt](test/Audio/CMakeLists.txt) and [test/Graphics/CMakeLists.txt](test/Graphics/CMakeLists.txt) both end with ~30 lines of resource-staging for iOS/Emscripten that is the same logic. Move the staging block into `zancle_add_test` ([cmake/Macros.cmake:493-592](cmake/Macros.cmake#L493-L592)) behind a new `RESOURCES` keyword (mirror what `zancle_add_example` already does at [Macros.cmake:454-473](cmake/Macros.cmake#L454-L473)). Each per-test file then becomes the 2-liner the others already are.

### 1.6 `zancle_add_library` is doing 11 things -- effort: medium

The macro mixes: target creation, emscripten options, public defines, C++23, coverage, warnings, visibility, PCH reuse, export-symbol naming, debug/release postfix, win32 .rc generation, version/soversion, Apple frameworks, iOS bundle props, install + export-set, pkg-config install, deferred export bookkeeping, include dirs, `ZA_STATIC` define, UTF-8 flag ([cmake/Macros.cmake:92-357](cmake/Macros.cmake#L92-L357)).

Refactor by splitting into composable functions, then have `zancle_add_library` call them in order:

```
zancle_add_library
  _zancle_create_module_target(...)
  _zancle_apply_common_compile(...)   # PUBLIC defines, C++23, warnings, vis, UTF-8
  _zancle_apply_pch(...)
  _zancle_apply_windows_rc(...)
  _zancle_apply_postfix_and_version(...)
  _zancle_apply_apple_packaging(...)
  _zancle_install_and_export(...)
  _zancle_apply_pdb(...)
```

Each is 10-30 lines, fits on a screen, has a single concern, can be reused (e.g. `_zancle_apply_pch` could service tests and examples instead of three near-identical PCH blocks at [Macros.cmake:152-159](cmake/Macros.cmake#L152-L159), [Macros.cmake:405-412](cmake/Macros.cmake#L405-L412), [Macros.cmake:509-515](cmake/Macros.cmake#L509-L515)).

## 2. Modernization opportunities

### 2.1 Drop `add_definitions` everywhere -- effort: small

`add_definitions` predates target-based CMake. Every use in this codebase ([CMakeLists.txt:48-244](CMakeLists.txt#L48-L244), [src/Zancle/GLUtils/CMakeLists.txt:18](src/Zancle/GLUtils/CMakeLists.txt#L18), [test/CMakeLists.txt:38-49](test/CMakeLists.txt#L38-L49)) should be `target_compile_definitions` on the new `zancle-build-options` INTERFACE target (see 1.1). The `add_compile_definitions` calls inside `ZA_ENABLE_STDLIB_ASSERTIONS` ([CMakeLists.txt:250-256](CMakeLists.txt#L250-L256)) are fine because they're explicitly used to propagate to subdirs, but they would also fit on the INTERFACE target.

### 2.2 Replace `${CMAKE_SYSTEM_NAME} STREQUAL "X"` with builtin predicates -- effort: small

[cmake/Config.cmake:2-67](cmake/Config.cmake#L2-L67) hand-rolls OS detection. Since CMake 3.25, you have `if(LINUX)`, `if(APPLE)`, `if(BSD)` directly. `WIN32`, `ANDROID`, `IOS`, `EMSCRIPTEN` are even older. The current chain still works but is noisier than:

```cmake
if(WIN32)
    set(ZA_OS_WINDOWS 1)
elseif(ANDROID)
    set(ZA_OS_ANDROID 1)
    set(ZA_OS_UNIX 1)
elseif(LINUX)
    set(ZA_OS_LINUX 1)
    set(ZA_OS_UNIX 1)
elseif(IOS)
    set(ZA_OS_IOS 1)
elseif(APPLE)
    set(ZA_OS_MACOS 1)
elseif(EMSCRIPTEN)
    ...
elseif(BSD)
    if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD") ...
endif()
```

Also: the `OPENGL_ES` variable is set in five different elseif branches to either 0 or 1; it could be derived once from `ZA_OS_ANDROID OR ZA_OS_IOS OR ZA_OS_EMSCRIPTEN` at the bottom of the file.

### 2.3 Stop bumping `CMAKE_POLICY_VERSION_MINIMUM "3.5"` -- effort: small

[CMakeLists.txt:2](CMakeLists.txt#L2) and [test/CMakeLists.txt:4](test/CMakeLists.txt#L4) both set this. As of CMake 4.x the policy-version backstop is now `3.10`; the `3.5` floor only matters for pre-CMake-4 third-party deps. If you have one specific offender (likely FreeType or MbedTLS), set this inside that dep's `block()` rather than globally -- it currently silences warnings on first-party code too.

### 2.4 `file(GLOB ...)` honesty -- effort: small to medium

The codebase is committed to `file(GLOB CONFIGURE_DEPENDS ...)` for sources, which is a defensible choice. Two specific cases where this is dishonest and you should switch to explicit lists:

- [src/Zancle/System/CMakeLists.txt:26-36](src/Zancle/System/CMakeLists.txt#L26-L36): globs everything then `list(FILTER ... EXCLUDE REGEX "/Unity/")`. The Unity-bundle pattern is load-bearing: forgetting to put a TU in `Unity/` would silently make it part of the standalone build. An explicit list per sub-module (one CMakeLists per topical module instead of one mega-glob) would be more honest -- and would naturally support per-module deps if you ever need them.
- [examples/bubble_idle/CMakeLists.txt:1-97](examples/bubble_idle/CMakeLists.txt#L1-L97): two nearly identical hardcoded source lists for the two arch branches. Compute a base list, then conditionally `list(APPEND ...)` the desktop-only files. Halves the maintenance burden.

### 2.5 `CMakeParseArguments` is deprecated as a module -- effort: trivial

[cmake/Macros.cmake:1](cmake/Macros.cmake#L1) does `include(CMakeParseArguments)`. `cmake_parse_arguments` has been a builtin since 3.5; the include is a no-op. Delete the line.

### 2.6 Use `target_sources` with `FILE_SET HEADERS` -- effort: medium

Modern install pattern: declare public headers via `target_sources(... PUBLIC FILE_SET zancle_public_headers TYPE HEADERS BASE_DIRS include FILES ...)`. This automates the `install(DIRECTORY include/ ...)` block at [CMakeLists.txt:325-328](CMakeLists.txt#L325-L328) and the framework PUBLIC_HEADER acrobatics at [CMakeLists.txt:339-433](CMakeLists.txt#L339-L433) (~95 lines that boil down to "copy each `include/Zancle/<Module>` into the framework Headers dir"). One `FILE_SET` per module replaces all those `add_custom_command(... cp -r ...)` POST_BUILD calls.

This is also the cleanest answer to "how do we discover/migrate Export.hpp" since FILE_SETs work uniformly with `generate_export_header`.

### 2.7 `set_target_properties` runs -- effort: trivial

There are 15+ separate `set_target_properties(${target} PROPERTIES X Y)` calls in [Macros.cmake](cmake/Macros.cmake) for the same target. CMake doesn't care, but a reader does. Collapse adjacent ones into a single multi-property call. Example: [Macros.cmake:171-209](cmake/Macros.cmake#L171-L209) -- four separate calls in the BUILD_SHARED branch could be one.

### 2.8 `if(POLICY CMP0168) cmake_policy(SET CMP0168 NEW)` -- effort: trivial

CMake 3.28 introduced CMP0168 and it's NEW as of 3.30. Since you've fixed the floor at 3.28, drop the `if(POLICY)` guard and just `cmake_policy(SET CMP0168 NEW)`. Same for [Audio/CMakeLists.txt:25-26](src/Zancle/Audio/CMakeLists.txt#L25-L26), [Graphics:39](src/Zancle/Graphics/CMakeLists.txt#L39), [Network:32](src/Zancle/Network/CMakeLists.txt#L32), [ImGui:34](src/Zancle/ImGui/CMakeLists.txt#L34): `set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)` is harmless but the policy was set in 3.13.

## 3. Readability cleanup

### 3.1 Naming consistency -- ZA_* vs ZANCLE_*

Mostly clean. `Zancle::<Module>` (CamelCase) is the public alias; `zancle-<module>` (lowercase-hyphen) is the underlying target; `ZA_*` is the option prefix. One stray: in `examples/bubble_idle` the macro is `BUBBLEBYTE_USE_STEAMWORKS` and the profiler macros are `SFEX_PROFILER_ENABLED` ([examples/CMakeLists.txt:67](examples/CMakeLists.txt#L67)). `SFEX_*` is leftover from "SFML examples" and should be `ZAEX_*` after the rebrand (note: this affects `Profiler.hpp` link-check tags). Low effort.

### 3.2 Dead/stale comments

- [Macros.cmake:75-85](cmake/Macros.cmake#L75-L85): docstring talks about "every Emscripten build" but the macro takes an optional visibility argument that's never used at any call site; if INTERFACE is never used, drop the `ARGV1` branch.
- Multiple commented-out tracy blocks ([Macros.cmake:115-121](cmake/Macros.cmake#L115-L121), [414-416](cmake/Macros.cmake#L414-L416), [517-519](cmake/Macros.cmake#L517-L519), [examples/bubble_idle:112-114](examples/bubble_idle/CMakeLists.txt#L112-L114)). Either bring tracy back as an option-gated block or remove the noise.
- [Audio/CMakeLists.txt:178-180](src/Zancle/Audio/CMakeLists.txt#L178-L180): a TODO that says "needs to be fixed when Zancle is used as a dependency" referring to a commented-out `target_include_directories`. Either fix it or convert to an issue/comment-only.
- Variable named `_sfml_emscripten_visibility` ([Macros.cmake:77](cmake/Macros.cmake#L77)) -- rebrand to `_zancle_*`.
- Inconsistent header-include glob: each module duplicates `"${INCROOT}/*.hpp" "${INCROOT}/*.inl" "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp"` literally. A helper `_zancle_glob_module_sources(INCROOT SRCROOT OUTVAR)` would tidy this up at the same time as 2.4.

### 3.3 Top-level CMakeLists ordering

[CMakeLists.txt](CMakeLists.txt) interleaves: option declarations, sanity checks, install-dir derivations, more options, CPack at the bottom. It's ~520 lines doing too many things. Reorganize into sections marked by `# ===` banners:

1. project + policies + global cache vars
2. options (all in one block, grouped by category)
3. derived configuration (install dirs, pkg-config dir, ZA_PDB_POSTFIX, etc.)
4. INTERFACE build-options target (sec 1.1)
5. third-party prefetches (`include(cmake/CPM.cmake)`, libbacktrace, mesa3d)
6. `add_subdirectory(src/Zancle)`
7. install/export rules + framework
8. extras gate + examples + tests + doc + format/tidy + cpack

Currently options appear in lines 25-26, 58-126, 130-130, 176, 180, 195, 232, 247, 260, 286, 454, 481, 487, 493, 496, 508, 510, 515. Hard to know which knobs exist without grepping.

### 3.4 `OPENGL_ES` (no prefix) leaking into the cache

[cmake/Config.cmake](cmake/Config.cmake) sets `OPENGL_ES` (no `ZA_` prefix); [CMakeLists.txt:114](CMakeLists.txt#L114) reads it as `${OPENGL_ES}` default. Should be `ZA_OPENGL_ES_DEFAULT` (or just drive from the OS booleans). The current name collides with anything else defining `OPENGL_ES`.

### 3.5 `ARCH_X86`/`ARCH_X64`/`ARCH_ARM64` are global non-prefixed -- effort: trivial

[cmake/Config.cmake:10-14](cmake/Config.cmake#L10-L14). Should be `ZA_ARCH_*` for consistency. `bubble_idle` already reads `ARCH_ARM64` ([bubble_idle:2](examples/bubble_idle/CMakeLists.txt#L2)) which is fine internally but pollutes user cmake namespace.

### 3.6 `set_public_symbols_hidden` is half-public, half-private -- effort: small

[Macros.cmake:7-16](cmake/Macros.cmake#L7-L16). The `if (NOT MINGW AND NOT ZA_BUILD_TEST_SUITE)` carve-out couples this helper to the test-suite gate, which is surprising. Document or move that policy decision next to the test-suite block.

### 3.7 Stale duplication for `find_package(Threads REQUIRED)` -- effort: trivial

Called in [CMakeLists.txt indirectly], [src/Zancle/CMakeLists.txt:?], [src/Zancle/Audio:145](src/Zancle/Audio/CMakeLists.txt#L145), [src/Zancle/Window:6](src/Zancle/Window/CMakeLists.txt#L6), [src/Zancle/System:45](src/Zancle/System/CMakeLists.txt#L45), [Macros.cmake (3 PCH branches)](cmake/Macros.cmake#L157), [test/CMakeLists.txt:29](test/CMakeLists.txt#L29), [examples/island:1](examples/island/CMakeLists.txt#L1). All redundant after the first call. Either call once at top level and rely on subsequent calls being no-ops (the current state, which is fine if intended), or make it an implicit dependency of the INTERFACE options target.

## 4. Reorganizeability friction

### 4.1 Adding a new module today requires touching 5+ files

If I want to split `Audio` into `Audio` + `AudioCodecs`, here's what I have to change:

1. `src/Zancle/AudioCodecs/CMakeLists.txt` -- new file mirroring Audio
2. `src/Zancle/CMakeLists.txt` -- new `if(ZA_BUILD_AUDIO_CODECS) add_subdirectory(...)` ([src/Zancle/CMakeLists.txt:10-49](src/Zancle/CMakeLists.txt#L10-L49))
3. Top-level `CMakeLists.txt` -- new `ZA_BUILD_AUDIO_CODECS` option + `add_definitions` + force-build logic ([CMakeLists.txt:81-110](CMakeLists.txt#L81-L110), [157-167](CMakeLists.txt#L157-L167))
4. `ZancleConfig.cmake.in` -- add to `ZA_SUPPORTED_COMPONENTS`, add to component-sort block ([cmake/ZancleConfig.cmake.in:72-117](cmake/ZancleConfig.cmake.in#L72-L117))
5. `tools/pkg-config/zancle-audiocodecs.pc.in` -- new file
6. `tools/pkg-config/zancle-all.pc.in` -- update aggregator
7. `Dependencies.cmake.in` -- new file
8. If frameworks, add a new `add_custom_command(TARGET Zancle POST_BUILD COMMAND cp -r ... AudioCodecs ...)` block ([CMakeLists.txt:378-433](CMakeLists.txt#L378-L433))
9. Top-level test-suite gate ([CMakeLists.txt:499](CMakeLists.txt#L499))
10. `test/CMakeLists.txt` -- new `add_subdirectory(AudioCodecs)` + runtests deps ([test/CMakeLists.txt:54-60](test/CMakeLists.txt#L54-L60), [154](test/CMakeLists.txt#L154))
11. Android push script ([test/CMakeLists.txt:71-90](test/CMakeLists.txt#L71-L90))

Most of this is mechanical and could be table-driven. The component-sort block in `ZancleConfig.cmake.in` ([cmake/ZancleConfig.cmake.in:99-117](cmake/ZancleConfig.cmake.in#L99-L117)) is the worst offender -- it open-codes the dependency graph as 7 separate `if(X IN_LIST)` blocks. Generate it from a list-of-lists at install time (write a `ZancleComponentGraph.cmake` via `configure_file`) and the per-module addition becomes a one-line list edit.

Suggested approach: define modules once at the top level

```cmake
set(ZA_MODULES
    System
    GLUtils      "ZA_BUILD_GLUTILS"      "System"
    Window       "ZA_BUILD_WINDOW"       "System;GLUtils"
    Network      "ZA_BUILD_NETWORK"      "System"
    Graphics     "ZA_BUILD_GRAPHICS"     "Window"
    ImGui        "ZA_BUILD_IMGUI"        "Graphics"
    Audio        "ZA_BUILD_AUDIO"        "System"
    Main         ""                      "")   # platform-conditional
```

Then drive: option declarations, `add_subdirectory` calls, force-build chains, pkg-config installs, ZancleConfig sort block, Android push, framework header copies, test runtests deps. The data already lives implicitly in [src/Zancle/CMakeLists.txt:10-49](src/Zancle/CMakeLists.txt#L10-L49); promote it.

### 4.2 The System aggregation makes 17 utility modules invisible to CMake

The recent modularization split utility code into 17 topical dirs under `include/Zancle/` and `src/Zancle/` ([src/Zancle/System/CMakeLists.txt:1-19](src/Zancle/System/CMakeLists.txt#L1-L19)), but a `file(GLOB_RECURSE)` reaggregates them. From CMake's perspective there's still one `zancle-system` target. Pros: keeps build/link simple, no SOVERSION explosion. Cons:

- Cannot link/test a single utility module in isolation.
- Cannot enforce internal layering (Algorithm not depending on IO, etc.) via CMake -- the rules can only live in header guards or `clang-tidy`.
- `target_compile_definitions` on `zancle-system` is the only knob; per-module options are impossible.
- Note: `include/Zancle/Chrono/` has NO corresponding `src/Zancle/Chrono/` directory, so the `MOD_SRCROOT` glob silently picks up nothing -- this is fine but not visible from the CMakeLists. An explicit list would surface that asymmetry.

Two cheap improvements without breaking the single-target story:

- Make `UTILITY_MODULES` the source-of-truth list and FAIL_IF the include or src dir is missing (catches typos when adding/removing).
- For each utility module, create an `OBJECT` library and link them into `zancle-system`. This lets you visualize the layering in IDE folders, and is the trivial step before promoting any one of them to a proper library target.

### 4.3 Renaming a target is a refactor

Anything that mentions `zancle-graphics` literally: [Macros.cmake (string concat)](cmake/Macros.cmake#L101), per-module `CMakeLists.txt` for `target_link_libraries`/`target_include_directories`, the foreach in `Macros.cmake` for export-set bookkeeping, pkg-config `.pc.in` filenames, Android push script, runtests deps, `ZancleConfig.cmake.in`. Anchoring everything on the `Zancle::Graphics` alias and using `$<TARGET_NAME:Zancle::Graphics>` where the actual target name is needed would localize most renames to `zancle_add_library`'s `string(TOLOWER ...)`.

### 4.4 The framework-build branch at [CMakeLists.txt:339-446](CMakeLists.txt#L339-L446) is a megablock

It's 100 lines of `add_custom_command(... COMMAND cp -r ... VERBATIM)` per module. Adding a new module means adding another nearly-identical 5-line block. This is the strongest argument for migrating to `FILE_SET HEADERS` (sec 2.6). As a quick interim, foreach over the module list (sec 4.1).

### 4.5 Examples opt-in is hardcoded

[examples/CMakeLists.txt:79-140](examples/CMakeLists.txt#L79-L140) is a tower of nested `if(ZA_BUILD_X)` / `if(NOT ZA_OS_IOS)` calls listing each example by name. Each example already self-describes its dependencies via `zancle_add_example(... DEPENDS ...)`. Two cleaner patterns:

1. `file(GLOB EXAMPLE_DIRS LIST_DIRECTORIES TRUE RELATIVE ${CMAKE_CURRENT_LIST_DIR} */)` then for each dir `add_subdirectory(${d})`; let each example's `CMakeLists.txt` self-guard with `if(NOT ZA_BUILD_GRAPHICS) return() endif()`.
2. Adopt a small data block at the top: `set(ZA_EXAMPLES network=ftp,http,sockets graphics=event_handling,stencil,...)` and drive `add_subdirectory` from it.

Option 1 is more idiomatic and adds zero per-example boilerplate.

## 5. Lower-priority observations

- [CMakeLists.txt:25-26](CMakeLists.txt#L25-L26): `zancle_set_option(CMAKE_BUILD_TYPE Release ...)` works, but the comment "has to be set before CMake detects/configures the toolchain" is wrong -- `project()` is called *after* this block at [CMakeLists.txt:29](CMakeLists.txt#L29). Toolchain detection happens at `project()`, so the order is fine; the comment is misleading.
- [CMakeLists.txt:312-318](CMakeLists.txt#L312-L318): the PCH-shared-libs and PCH-macOS fatal errors are checked late, after lots of derived configuration. Move adjacent to where `ZA_ENABLE_PCH` is defined.
- [Macros.cmake:53-56](cmake/Macros.cmake#L53-L56): iOS bundle identifier uses `org.zancle.${target}` but the target name is `zancle-graphics`, producing `org.zancle.zancle-graphics`. Probably a bug across the rebrand (used to be `org.sfml.${target}`).
- [Macros.cmake:266](cmake/Macros.cmake#L266): same -- `org.zancle.${target}` where `${target}` is lowercase-hyphen.
- [src/Zancle/Audio/CMakeLists.txt:149](src/Zancle/Audio/CMakeLists.txt#L149): `${CODECS_SRC}` is never defined. Dead concat. Probably leftover.
- [src/Zancle/Network/CMakeLists.txt:41-42](src/Zancle/Network/CMakeLists.txt#L41-L42): `# TODO P0: change to use CPM` -- the rest of the project uses `CPMAddPackage`, this one outlier still uses `FetchContent_Declare`/`FetchContent_MakeAvailable`. Convert.
- [src/Zancle/ImGui/CMakeLists.txt:89-90](src/Zancle/ImGui/CMakeLists.txt#L89-L90): `# TODO P0: this is hardcoded, should follow same pattern as Macros.cmake` -- `install(TARGETS imgui EXPORT ZancleImGuiStaticTargets ...)` ignores the Static/Shared config_name distinction in `zancle_export_targets()`. This will mis-install when `BUILD_SHARED_LIBS=ON`.
- [src/Zancle/ImGui/CMakeLists.txt:50](src/Zancle/ImGui/CMakeLists.txt#L50): `IMGUI_SOURCES = ${imgui_sources} ${imgui_sources_cpp}` -- `imgui_sources_cpp` is never defined. Harmless concat with empty.
- [src/Zancle/CMakeLists.txt:52-67](src/Zancle/CMakeLists.txt#L52-L67): the libatomic check is duplicated near-identically inside Window's CMakeLists ([Window:158-165](src/Zancle/Window/CMakeLists.txt#L158-L165)). Pick one place.
- [CMakePresets.json](CMakePresets.json): 20 presets, several only differ by a flag tweak. Specifically the `*_no_time_trace` variants ([CMakePresets.json:166-184](CMakePresets.json#L166-L184)) only override an env var; consider documenting that users can do `CMAKE_GENERATOR_TOOLSET_*` env overrides directly. The Windows-hardcoded paths (`C:/msys64/...`) at [CMakePresets.json:95](CMakePresets.json#L95) and [189](CMakePresets.json#L189) belong in a personal `CMakeUserPresets.json` -- they break the file on non-Windows boxes.
- [CMakePresets.json:2](CMakePresets.json#L2): `"version": 3` is from CMake 3.21. You can bump to 6 (CMake 3.25+) and use `include` to split into per-platform files, removing the cross-platform-conflict problem.
- [src/Zancle/System/CMakeLists.txt:1-19](src/Zancle/System/CMakeLists.txt#L1-L19): the `UTILITY_MODULES` list is ordered "Base, Trait, Math, Diagnostic, Vocabulary, Container, Algorithm, String, ..." -- this looks bottom-up in the layering. Worth a comment saying so, or alphabetize it. Adding a module risks inserting it at the wrong spot if the order matters.
- [cmake/Modules/FindFreetype.cmake](cmake/Modules/FindFreetype.cmake) (163 lines), [FindMbedTLS.cmake](cmake/Modules/FindMbedTLS.cmake) (140 lines), [FindVorbis.cmake](cmake/Modules/FindVorbis.cmake) (69 lines) -- haven't been audited but the lengths suggest they may predate CMake's built-in modules. CMake 3.28 ships `FindFreetype` and `FindVorbis`-equivalents. Worth a sweep.
- [test/install/CMakeLists.txt:1-2](test/install/CMakeLists.txt#L1-L2): its own `cmake_minimum_required(VERSION 3.28)` and `project()` -- correct for a `find_package`-driven install test, but the version is duplicated; consider hoisting to a top-level `ZA_CMAKE_VERSION` variable so bumps are one-touch.
- [test/CMakeLists.txt:154](test/CMakeLists.txt#L154): `add_custom_target(runtests DEPENDS test-zancle-base test-zancle-system ...)` -- 7-target hardcoded list; auto-generate from a single source.
- [examples/CMakeLists.txt:55-75](examples/CMakeLists.txt#L55-L75): the `ExampleProfilerEnabled`/`Disabled` mechanism is clever but the `if(${enable_macro})` style is fragile (bare-word truthy check). Use `if(enable_macro)` (no `${}`) to check the function argument directly.

## 6. Risks and non-issues

- **`file(GLOB CONFIGURE_DEPENDS)` everywhere**: defensible. With `CONFIGURE_DEPENDS` CMake re-checks the glob at build time, paying a small cost in exchange for not having to edit CMakeLists when adding a file. Given the project's velocity, this is the right tradeoff. Don't switch to explicit lists *just* for purity; only switch where the glob is misleading (sec 2.4).
- **`zancle_set_option` macro at [CMakeLists.txt:10-15](CMakeLists.txt#L10-L15)**: small but it sidesteps an `option()` quirk where you can't reasonably change defaults from a parent project. Keep it.
- **Manual OS detection at [cmake/Config.cmake](cmake/Config.cmake)**: modernization opportunity (sec 2.2) but not actually broken. It produces flags the rest of the code reads. Don't rip it out without a sweep.
- **CPM over FetchContent**: CPM gives caching and a uniform option-passing surface; the project is well-served by it. Don't switch back to vanilla FetchContent. (One exception: the MbedTLS block at [Network:29-52](src/Zancle/Network/CMakeLists.txt#L29-L52) uses `FetchContent` -- migrate this *to* CPM, not the other way.)
- **`set_target_properties(... INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ...)` for SDL3** at [Window:46-50](src/Zancle/Window/CMakeLists.txt#L46-L50): looks weird but is the correct workaround for getting `-isystem` treatment from a CPM dep that doesn't mark its own headers SYSTEM. Keep.
- **Per-target UTF-8 flag for MSVC**: a one-line `target_compile_options(${target} PRIVATE /utf-8)` in each of the three macros looks like duplication, but it's gated on `ZA_COMPILER_MSVC`, and you can't add it once globally without leaking into third-party MSVC builds. Acceptable.
- **The `zancle_add_test` `add_test(NAME ${target} COMMAND ${target})` pattern**: looks like it could use `catch_discover_tests` but the comment at [Macros.cmake:579-585](cmake/Macros.cmake#L579-L585) explains the bespoke runner. Keep.
- **`Macros.cmake` is a macro and not a function**: macros leak variables; the file uses both. Most callers don't care, but the `THIS_*` parsed args are guaranteed to be in caller scope which is occasionally relied on. Don't blindly flip to functions.

## Appendix: file inventory

| Path | Role |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Top-level: options, platform detection coordination, framework build, CPack, examples/test/doc gates |
| [CMakePresets.json](CMakePresets.json) | 20 build presets (clang/gcc/emcc, debug/release, sanitizer, arm64) |
| [cmake/Config.cmake](cmake/Config.cmake) | OS + compiler detection; sets `ZA_OS_*`/`ZA_COMPILER_*`/`OPENGL_ES`; emscripten flag matrix |
| [cmake/Macros.cmake](cmake/Macros.cmake) | The 350-line `zancle_add_library` + `zancle_add_example` + `zancle_add_test` + `zancle_export_targets`; helpers `zancle_set_stdlib`, `zancle_apply_emscripten_options`, `set_public_symbols_hidden` |
| [cmake/CompilerWarnings.cmake](cmake/CompilerWarnings.cmake) | `set_target_warnings(target)` -- MSVC/GCC/Clang warning lists |
| [cmake/CPM.cmake](cmake/CPM.cmake) | Vendored CPM.cmake (1363 lines, third-party) |
| [cmake/Format.cmake](cmake/Format.cmake) | clang-format runner (invoked by `make format`) |
| [cmake/Tidy.cmake](cmake/Tidy.cmake) | clang-tidy runner (invoked by `make tidy`) |
| [cmake/Mesa3D.cmake](cmake/Mesa3D.cmake) | Optional download + install of Mesa3D DLLs on Windows for software-GL testing |
| [cmake/ZancleConfig.cmake.in](cmake/ZancleConfig.cmake.in) | Installed `find_package(Zancle)` entry point: component-sort, static/shared selection, dep includes |
| [cmake/Modules/FindDRM.cmake](cmake/Modules/FindDRM.cmake) | Find DRM (Linux DRM windowing backend) |
| [cmake/Modules/FindEGL.cmake](cmake/Modules/FindEGL.cmake) | Find EGL (with emscripten special-case) |
| [cmake/Modules/FindGBM.cmake](cmake/Modules/FindGBM.cmake) | Find GBM (DRM companion) |
| [cmake/Modules/FindGLES.cmake](cmake/Modules/FindGLES.cmake) | Find OpenGL ES (with emscripten special-case) |
| [cmake/Modules/FindUDev.cmake](cmake/Modules/FindUDev.cmake) | Find UDev (Linux input device discovery) |
| [cmake/Modules/FindFreetype.cmake](cmake/Modules/FindFreetype.cmake) | Local FindFreetype override (predates CMake's) |
| [cmake/Modules/FindFLAC.cmake](cmake/Modules/FindFLAC.cmake) | Find FLAC (system-deps build path) |
| [cmake/Modules/FindVorbis.cmake](cmake/Modules/FindVorbis.cmake) | Find Vorbis (system-deps build path) |
| [cmake/Modules/FindMbedTLS.cmake](cmake/Modules/FindMbedTLS.cmake) | Find MbedTLS (system-deps build path) |
| [cmake/Modules/FindImGui.cmake](cmake/Modules/FindImGui.cmake) | Find ImGui sources (system-deps build path) |
| [src/Zancle/CMakeLists.txt](src/Zancle/CMakeLists.txt) | Includes Macros.cmake, gates and `add_subdirectory()`s each module, libatomic check |
| [src/Zancle/System/CMakeLists.txt](src/Zancle/System/CMakeLists.txt) | Aggregates 17 utility submodules into `zancle-system`; PCH source for the rest |
| [src/Zancle/System/Dependencies.cmake.in](src/Zancle/System/Dependencies.cmake.in) | Installed find_dep template (Threads only) |
| [src/Zancle/Main/CMakeLists.txt](src/Zancle/Main/CMakeLists.txt) | Tiny platform-entry-point library (Win32/iOS/Android) |
| [src/Zancle/GLUtils/CMakeLists.txt](src/Zancle/GLUtils/CMakeLists.txt) | GL helpers, EGL/desktop-GL selection, glad/vulkan headers |
| [src/Zancle/GLUtils/Dependencies.cmake.in](src/Zancle/GLUtils/Dependencies.cmake.in) | Installed find_dep template |
| [src/Zancle/Window/CMakeLists.txt](src/Zancle/Window/CMakeLists.txt) | Windowing, SDL3 CPM (or local), DRM, OpenGL selection |
| [src/Zancle/Window/Dependencies.cmake.in](src/Zancle/Window/Dependencies.cmake.in) | Installed find_dep template |
| [src/Zancle/Graphics/CMakeLists.txt](src/Zancle/Graphics/CMakeLists.txt) | Graphics + FreeType CPM, stb_image, qoi |
| [src/Zancle/Graphics/Dependencies.cmake.in](src/Zancle/Graphics/Dependencies.cmake.in) | Installed find_dep template |
| [src/Zancle/ImGui/CMakeLists.txt](src/Zancle/ImGui/CMakeLists.txt) | ImGui CPM + custom `imgui` STATIC target |
| [src/Zancle/Audio/CMakeLists.txt](src/Zancle/Audio/CMakeLists.txt) | Audio + Ogg/FLAC/Vorbis CPM, miniaudio, minimp3 |
| [src/Zancle/Audio/Dependencies.cmake.in](src/Zancle/Audio/Dependencies.cmake.in) | Installed find_dep template |
| [src/Zancle/Network/CMakeLists.txt](src/Zancle/Network/CMakeLists.txt) | Network + MbedTLS FetchContent (the lone FetchContent holdout) |
| [src/Zancle/Network/Dependencies.cmake.in](src/Zancle/Network/Dependencies.cmake.in) | Installed find_dep template |
| [test/CMakeLists.txt](test/CMakeLists.txt) | Test root: `zancle-test-main` static lib, `runtests` custom target, Android push script, OpenCppCoverage glue |
| [test/install/CMakeLists.txt](test/install/CMakeLists.txt) | Standalone consumer that `find_package(Zancle)` to verify install |
| [test/Audio,Base,GLUtils,Graphics,Network,System,Window/CMakeLists.txt](test/) | Per-module test entries; mostly one-line `zancle_add_test`; Audio/Graphics carry resource-bundling boilerplate |
| [examples/CMakeLists.txt](examples/CMakeLists.txt) | `ExampleUtils` + `ExampleProfilerEnabled/Disabled` libs, then nested `add_subdirectory()` chain for each example |
| [examples/<name>/CMakeLists.txt](examples/) | Per-example: source list + `zancle_add_example(name DEPENDS ...)`; some carry iOS resource bundles or non-Zancle CPM deps (box2d, steam) |
| [examples/android/app/src/main/jni/CMakeLists.txt](examples/android/app/src/main/jni/CMakeLists.txt) | Android NDK glue (not reviewed in detail) |
| [doc/CMakeLists.txt](doc/CMakeLists.txt) | Doxygen integration |
| [tools/flac/PatchFLAC.cmake](tools/flac/PatchFLAC.cmake), `PatchOgg`, `PatchVorbis`, `PatchFreetype`, `PatchMbedTLS`, `PatchTidy` | Sed-style scripts run via `-P` on third-party CMakeLists to patch them post-CPM-fetch |
