# include the compiler warnings helpers
include(${CMAKE_CURRENT_LIST_DIR}/CompilerWarnings.cmake)

# helper function to tweak visibility of public symbols
function(set_public_symbols_hidden target)
    # this breaks explicit template instantiations, see GCC bug #109387
    # checking test suite macro for Catch PCH
    if (NOT MINGW AND NOT ZA_BUILD_TEST_SUITE)
        # ensure public symbols are hidden by default (exported ones are explicitly marked)
        set_target_properties(${target} PROPERTIES
                              CXX_VISIBILITY_PRESET hidden
                              VISIBILITY_INLINES_HIDDEN YES)
    endif()
endfunction()

# This little macro lets you set any Xcode specific property
macro(zancle_set_xcode_property TARGET XCODE_PROPERTY XCODE_VALUE)
    set_property(TARGET ${TARGET} PROPERTY XCODE_ATTRIBUTE_${XCODE_PROPERTY} ${XCODE_VALUE})
endmacro()

# set the appropriate standard library on each platform for the given target
# example: zancle_set_stdlib(zancle-system)
function(zancle_set_stdlib target)
    # for gcc on Windows, apply the ZA_USE_STATIC_STD_LIBS option if it is enabled
    if(ZA_OS_WINDOWS)
        if(ZA_COMPILER_GCC)
            if(ZA_USE_STATIC_STD_LIBS AND NOT ZA_COMPILER_GCC_TDM)
                target_link_libraries(${target} PRIVATE "-static-libgcc" "-static-libstdc++")
            elseif(NOT ZA_USE_STATIC_STD_LIBS AND ZA_COMPILER_GCC_TDM)
                target_link_libraries(${target} PRIVATE "-shared-libgcc" "-shared-libstdc++")
            endif()
        elseif(ZA_COMPILER_MSVC)
            if(ZA_USE_STATIC_STD_LIBS)
                set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            endif()
        endif()
    endif()
endfunction()

function(zancle_set_common_ios_properties target)
    # enable automatic reference counting on iOS
    zancle_set_xcode_property(${target} CLANG_ENABLE_OBJC_ARC YES)
    zancle_set_xcode_property(${target} IPHONEOS_DEPLOYMENT_TARGET "${ZA_IOS_DEPLOYMENT_TARGET}")
    zancle_set_xcode_property(${target} CODE_SIGN_IDENTITY "${ZA_CODE_SIGN_IDENTITY}")

    get_target_property(target_type ${target} TYPE)
    if(target_type STREQUAL "EXECUTABLE")
        set_target_properties(${target} PROPERTIES
            MACOSX_BUNDLE ON # Bare executables are not usable on iOS, only bundle applications
            MACOSX_BUNDLE_GUI_IDENTIFIER "org.zancle.${target}" # If missing, trying to launch an example in simulator will make Xcode < 9.3 crash
            MACOSX_BUNDLE_BUNDLE_NAME "${target}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
        )
    endif()
endfunction()

# Apply the Emscripten-specific compile and link options to `target`.
#
# Required for any TU that participates in a `--shared-memory` link (i.e.
# every Emscripten build of Zancle): the options pull in `-pthread`, which
# enables the `atomics` and `bulk-memory` Wasm features. Without them,
# `wasm-ld` rejects the resulting `.o` files at link time with
# `--shared-memory is disallowed because it was not compiled with 'atomics'
# or 'bulk-memory' features`.
#
# On non-Emscripten builds this is a no-op.
macro(zancle_apply_emscripten_options target)
    if(ZA_OS_EMSCRIPTEN)
        target_compile_options(${target} PRIVATE ${ZA_EMSCRIPTEN_TARGET_COMPILE_OPTIONS})
        target_link_options(${target} PRIVATE ${ZA_EMSCRIPTEN_TARGET_LINK_OPTIONS})
    endif()
endmacro()


# Apply the standard "third-party hygiene" treatment to a set of targets
# pulled in by CPM or FetchContent. This consolidates the per-module boilerplate
# previously inlined in Audio/Graphics/ImGui/Network.
#
# Each target receives:
#   - FOLDER "Dependencies" (IDE grouping)
#   - SYSTEM ON (include dirs treated as -isystem)
#   - POSITION_INDEPENDENT_CODE ON when BUILD_SHARED_LIBS is ON
#   - target_compile_options PRIVATE -w (silence third-party warnings)
#   - zancle_set_stdlib + zancle_apply_emscripten_options
#
# Optional UNITY_BUILD keyword turns on unity build with batch size 256;
# otherwise unity build is forced OFF.
#
# Missing targets are silently skipped, so the helper is safe to call when a
# target is gated behind a feature flag.
#
# Usage:
#   zancle_tame_thirdparty(TARGETS foo bar baz)
#   zancle_tame_thirdparty(UNITY_BUILD TARGETS foo bar)
#
# IMPORTANT: call this OUTSIDE any block() / function() scope that overrode
# BUILD_SHARED_LIBS, so the helper sees the original (project-wide) value.
function(zancle_tame_thirdparty)
    cmake_parse_arguments(ZTT "UNITY_BUILD" "" "TARGETS" ${ARGN})

    foreach(t IN LISTS ZTT_TARGETS)
        if(NOT TARGET ${t})
            continue()
        endif()

        set_target_properties(${t} PROPERTIES
            FOLDER "Dependencies"
            SYSTEM ON
        )

        if(BUILD_SHARED_LIBS)
            set_target_properties(${t} PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif()

        if(ZTT_UNITY_BUILD)
            set_target_properties(${t} PROPERTIES
                UNITY_BUILD ON
                UNITY_BUILD_BATCH_SIZE 256
            )
        else()
            set_target_properties(${t} PROPERTIES UNITY_BUILD OFF)
        endif()

        target_compile_options(${t} PRIVATE -w)

        zancle_set_stdlib(${t})
        zancle_apply_emscripten_options(${t})
    endforeach()
endfunction()


# -- zancle_add_library helpers ------------------------------------------------
#
# Each helper handles one concern of a Zancle library target. The public
# `zancle_add_library` macro composes them in order. Helpers are functions
# (their writes don't leak) but they freely READ caller-scope variables
# (`THIS_STATIC`, `THIS_DEPENDENCIES`, `module`, etc.) set by the macro's
# `cmake_parse_arguments` call.

# Create the library target itself, set up universal compile concerns
# (warnings, visibility, build-options link, coverage, DEFINE_SYMBOL/EXPORT_NAME).
function(_zancle_create_module_target target module)
    if(THIS_STATIC)
        add_library(${target} STATIC ${THIS_SOURCES})
    else()
        add_library(${target} ${THIS_SOURCES})
    endif()

    add_library(Zancle::${module} ALIAS ${target})

    # Emscripten compile/link options + .html output suffix.
    zancle_apply_emscripten_options(${target})
    if(ZA_OS_EMSCRIPTEN)
        set_target_properties(${target} PROPERTIES SUFFIX ".html")
    endif()

    # All Zancle build-time toggles (incl. C++23 feature requirement) live on a
    # single INTERFACE target; PUBLIC-link wrapped in $<BUILD_INTERFACE:...> so
    # the install/export set is unaffected.
    target_link_libraries(${target} PUBLIC $<BUILD_INTERFACE:zancle-build-options>)

    # GCC/Clang coverage flags
    if(ZA_ENABLE_COVERAGE AND (ZA_COMPILER_GCC OR ZA_COMPILER_CLANG))
        target_compile_options(${target} PUBLIC
            $<$<CONFIG:DEBUG>:-O0>
            $<$<CONFIG:DEBUG>:-g>
            $<$<CONFIG:DEBUG>:-fprofile-arcs>
            $<$<CONFIG:DEBUG>:-ftest-coverage>)
        target_link_options(${target} PUBLIC $<$<CONFIG:DEBUG>:--coverage>)
    endif()

    set_target_warnings(${target})
    set_public_symbols_hidden(${target})

    # DEFINE_SYMBOL is what CMake uses for the dllexport macro when building shared.
    string(REPLACE "-" "_" NAME_UPPER "${target}")
    string(TOUPPER "${NAME_UPPER}" NAME_UPPER)

    set_target_properties(${target} PROPERTIES
        DEFINE_SYMBOL ${NAME_UPPER}_EXPORTS
        EXPORT_NAME   Zancle::${module}
    )
endfunction()

# PCH reuse from zancle-system (the umbrella target carrying the PCH source).
function(_zancle_apply_pch target)
    if(NOT ZA_ENABLE_PCH OR ${target} STREQUAL "zancle-system")
        return()
    endif()
    message(VERBOSE "enabling PCH for Zancle library '${target}'")
    target_precompile_headers(${target} REUSE_FROM zancle-system)
    target_link_libraries(${target} PRIVATE Threads::Threads)
endfunction()

# Windows shared-library .rc resource generation, prefix/suffix tweaks.
function(_zancle_apply_windows_rc target module)
    if(NOT (BUILD_SHARED_LIBS AND NOT THIS_STATIC AND ZA_OS_WINDOWS))
        return()
    endif()

    set_target_properties(${target} PROPERTIES
        DEBUG_POSTFIX -d
        SUFFIX        "-${PROJECT_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )

    # fill in template variables for resource.rc.in
    string(TIMESTAMP RC_CURRENT_YEAR "%Y")
    set(RC_MODULE_NAME "${module}")
    set(RC_VERSION_SUFFIX "")           # Add something like the git revision short SHA-1 in the future
    set(RC_PRERELEASE "0")              # Set to 1 to mark the DLL as a pre-release DLL
    set(RC_TARGET_NAME "${target}")
    set(RC_TARGET_FILE_NAME_SUFFIX "-${PROJECT_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")

    configure_file(
        "${PROJECT_SOURCE_DIR}/tools/windows/resource.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${target}.rc"
        @ONLY
    )

    target_sources(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${target}.rc")

    # gcc/clang on Windows: strip "lib" prefix and use .a for import libs
    if(ZA_COMPILER_GCC OR ZA_COMPILER_CLANG)
        set_target_properties(${target} PROPERTIES
            PREFIX        ""
            IMPORT_SUFFIX ".a"
        )
    endif()
endfunction()

# DEBUG_POSTFIX/RELEASE_POSTFIX, SOVERSION/VERSION, IDE folder, stdlib selection.
function(_zancle_apply_postfix_and_version target)
    # Windows shared builds already set DEBUG_POSTFIX in _zancle_apply_windows_rc.
    if(BUILD_SHARED_LIBS AND NOT THIS_STATIC)
        if(NOT ZA_OS_WINDOWS)
            set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -d)
        endif()
    else()
        set_target_properties(${target} PROPERTIES
            DEBUG_POSTFIX          -s-d
            RELEASE_POSTFIX        -s
            MINSIZEREL_POSTFIX     -s
            RELWITHDEBINFO_POSTFIX -s
        )
    endif()

    # SOVERSION/VERSION (Android strips these so skip there)
    if(NOT ZA_OS_ANDROID)
        set_target_properties(${target} PROPERTIES
            SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            VERSION   ${PROJECT_VERSION}
        )
    endif()

    set_target_properties(${target} PROPERTIES FOLDER "Zancle")
    zancle_set_stdlib(${target})
endfunction()

# Apple framework/bundle props and Android PIC requirement.
function(_zancle_apply_apple_packaging target)
    if((ZA_OS_MACOS OR ZA_OS_IOS) AND BUILD_SHARED_LIBS AND NOT THIS_STATIC)
        if(ZA_BUILD_FRAMEWORKS)
            set_target_properties(${target} PROPERTIES
                FRAMEWORK                             ON
                FRAMEWORK_VERSION                     ${PROJECT_VERSION}
                MACOSX_FRAMEWORK_IDENTIFIER           org.zancle.${target}
                MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
                MACOSX_FRAMEWORK_BUNDLE_VERSION       ${PROJECT_VERSION}
            )
        endif()

        # Let dylibs/frameworks ship inside user app bundles (only if no rpath options set).
        if(NOT CMAKE_SKIP_RPATH
           AND NOT CMAKE_SKIP_INSTALL_RPATH
           AND NOT CMAKE_INSTALL_RPATH
           AND NOT CMAKE_INSTALL_RPATH_USE_LINK_PATH
           AND NOT CMAKE_INSTALL_NAME_DIR)
            set_target_properties(${target} PROPERTIES INSTALL_NAME_DIR "@rpath")
            if(NOT CMAKE_SKIP_BUILD_RPATH)
                set_target_properties(${target} PROPERTIES BUILD_WITH_INSTALL_NAME_DIR ON)
            endif()
        endif()
    endif()

    if(ZA_OS_IOS)
        zancle_set_common_ios_properties(${target})
    endif()

    # On Android everything ends up in a shared library, so static archives must
    # still be position-independent code.
    if(ZA_OS_ANDROID)
        set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
endfunction()

# Visual-Studio PDB output naming/placement.
function(_zancle_apply_pdb target)
    if(NOT ZA_GENERATE_PDB)
        return()
    endif()

    # PDB files are only generated in Debug and RelWithDebInfo configurations.
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set(pdb_postfix "-d")
    else()
        set(pdb_postfix "")
    endif()

    if(BUILD_SHARED_LIBS AND NOT THIS_STATIC)
        # DLLs use the linker PDB.
        set_target_properties(${target} PROPERTIES
            PDB_NAME             "${target}${pdb_postfix}"
            PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
        )
    else()
        if(NOT ${target} STREQUAL "zancle-main")
            string(PREPEND pdb_postfix "-s")
        endif()

        if(ZA_ENABLE_PCH)
            # PCH reuse forces the PDB name to match the reused target.
            message(VERBOSE "overriding PDB name for '${target}' with \"zancle-system\" due to PCH being enabled")
            set_target_properties(${target} PROPERTIES
                COMPILE_PDB_NAME             "zancle-system"
                COMPILE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
            )
        else()
            # Static libs have no linker PDB; use the compiler PDB.
            set_target_properties(${target} PROPERTIES
                COMPILE_PDB_NAME             "${target}${pdb_postfix}"
                COMPILE_PDB_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
            )
        endif()
    endif()
endfunction()

# install(TARGETS), pkg-config, Dependencies.cmake.in, ZA_STATIC, include dirs,
# UTF-8 source flag, and global module-bookkeeping for zancle_export_targets().
function(_zancle_install_and_export target module)
    if(BUILD_SHARED_LIBS)
        set(config_name "Shared")
    else()
        set(config_name "Static")
    endif()

    # Declare the module's public headers as a FILE_SET. Each module installs
    # only its own `include/Zancle/<Module>/` tree; disabling a module via
    # -DZA_BUILD_X=OFF therefore omits its headers from the install, which the
    # older `install(DIRECTORY include/ ...)` dump-everything pattern could
    # not do.
    #
    # Special-case "System": that target aggregates the 17 flat utility
    # modules listed in `UTILITY_MODULES` (set by src/Zancle/System/
    # CMakeLists.txt before it calls zancle_add_library), so glob across all
    # of them. System also owns the four top-level umbrella headers
    # (Config/OpenGL/GpuPreference/Main).
    #
    # Framework builds keep the existing add_custom_command-based POST_BUILD
    # copy in the top-level CMakeLists, so the FILE_SET install line below is
    # gated on NOT ZA_BUILD_FRAMEWORKS.
    set(_module_headers "")
    if(${module} STREQUAL "System")
        foreach(_um IN LISTS UTILITY_MODULES)
            file(GLOB_RECURSE _um_headers CONFIGURE_DEPENDS
                "${PROJECT_SOURCE_DIR}/include/Zancle/${_um}/*.hpp"
                "${PROJECT_SOURCE_DIR}/include/Zancle/${_um}/*.inl"
            )
            list(APPEND _module_headers ${_um_headers})
        endforeach()
        list(APPEND _module_headers
            "${PROJECT_SOURCE_DIR}/include/Zancle/Config.hpp"
            "${PROJECT_SOURCE_DIR}/include/Zancle/OpenGL.hpp"
            "${PROJECT_SOURCE_DIR}/include/Zancle/GpuPreference.hpp"
            "${PROJECT_SOURCE_DIR}/include/Zancle/Main.hpp"
        )
    else()
        file(GLOB_RECURSE _module_headers CONFIGURE_DEPENDS
            "${PROJECT_SOURCE_DIR}/include/Zancle/${module}/*.hpp"
            "${PROJECT_SOURCE_DIR}/include/Zancle/${module}/*.inl"
        )
    endif()
    # BASE_DIRS = include so headers install under <prefix>/include/Zancle/...
    # (the framework branch uses include/Zancle/ instead -- the framework
    # bundle absorbs the Zancle/ segment into its Headers/ dir).
    target_sources(${target} PUBLIC
        FILE_SET zancle_public_headers
        TYPE HEADERS
        BASE_DIRS "${PROJECT_SOURCE_DIR}/include"
        FILES ${_module_headers}
    )

    if(ZA_BUILD_FRAMEWORKS)
        install(TARGETS ${target} EXPORT Zancle${module}${config_name}Targets
                RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
                LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT bin
                ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT devel
                FRAMEWORK DESTINATION "."                     COMPONENT bin)
    else()
        install(TARGETS ${target} EXPORT Zancle${module}${config_name}Targets
                RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}      COMPONENT bin
                LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR}      COMPONENT bin
                ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR}      COMPONENT devel
                FRAMEWORK DESTINATION "."                          COMPONENT bin
                FILE_SET zancle_public_headers
                          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}  COMPONENT devel)
    endif()

    # pkg-config
    if(ZA_INSTALL_PKGCONFIG_FILES AND NOT ${target} STREQUAL "zancle-main")
        configure_file(
            "${PROJECT_SOURCE_DIR}/tools/pkg-config/${target}.pc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/tools/pkg-config/${target}.pc"
            @ONLY)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tools/pkg-config/${target}.pc"
                DESTINATION "${ZA_PKGCONFIG_INSTALL_DIR}")
    endif()

    # Frameworks must be set up before any install(EXPORT) call, so defer
    # install(EXPORT) to zancle_export_targets(). Track modules added so far.
    get_property(ZA_ADD_LIBRARY_MODULES GLOBAL PROPERTY ZA_ADD_LIBRARY_MODULES_PROPERTY)
    list(APPEND ZA_ADD_LIBRARY_MODULES ${module})
    set_property(GLOBAL PROPERTY ZA_ADD_LIBRARY_MODULES_PROPERTY "${ZA_ADD_LIBRARY_MODULES}")

    # Generate per-module Dependencies.cmake when static linking.
    if(NOT BUILD_SHARED_LIBS AND THIS_DEPENDENCIES)
        include(CMakePackageConfigHelpers)
        configure_package_config_file("${THIS_DEPENDENCIES}"
            "${CMAKE_CURRENT_BINARY_DIR}/Zancle${module}Dependencies.cmake"
            INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zancle")
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Zancle${module}Dependencies.cmake"
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Zancle
                COMPONENT devel)
    endif()

    target_include_directories(${target}
        PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        PRIVATE ${PROJECT_SOURCE_DIR}/src)

    if(ZA_BUILD_FRAMEWORKS)
        target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:Zancle.framework>)
    else()
        target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:include>)
    endif()

    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${target} PUBLIC "ZA_STATIC")
    endif()

    if(ZA_COMPILER_MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
    endif()
endfunction()


# add a new target which is a Zancle library
# example: zancle_add_library(Graphics
#                           SOURCES sprite.cpp image.cpp ...
#                           [STATIC]) # Always create a static library and ignore BUILD_SHARED_LIBS
macro(zancle_add_library module)
    cmake_parse_arguments(THIS "STATIC" "DEPENDENCIES" "SOURCES" ${ARGN})
    if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Extra unparsed arguments when calling zancle_add_library: ${THIS_UNPARSED_ARGUMENTS}")
    endif()

    string(TOLOWER zancle-${module} target)

    _zancle_create_module_target(${target} ${module})
    _zancle_apply_pch(${target})
    _zancle_apply_windows_rc(${target} ${module})
    _zancle_apply_postfix_and_version(${target})
    _zancle_apply_apple_packaging(${target})
    _zancle_apply_pdb(${target})
    _zancle_install_and_export(${target} ${module})
endmacro()

# add a new target which is a Zancle example
# example: zancle_add_example(ftp
#                           SOURCES ftp.cpp ...
#                           BUNDLE_RESOURCES MainMenu.nib ...    # Files to be added in target but not installed next to the executable
#                           DEPENDS Zancle::Network)
macro(zancle_add_example target)

    # parse the arguments
    cmake_parse_arguments(THIS "GUI_APP" "" "SOURCES;BUNDLE_RESOURCES;DEPENDS" ${ARGN})

    # check whether resources must be added in target
    set(target_input ${THIS_SOURCES})
    if(THIS_BUNDLE_RESOURCES)
        set(target_input ${target_input} ${THIS_BUNDLE_RESOURCES})
    endif()

    # create the target
    if(THIS_GUI_APP AND ZA_OS_WINDOWS AND NOT DEFINED CMAKE_CONFIGURATION_TYPES AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
        add_executable(${target} WIN32 ${target_input})
        target_link_libraries(${target} PRIVATE Zancle::Main)
    elseif(THIS_GUI_APP AND ZA_OS_IOS)

        # For iOS apps we need the launch screen storyboard,
        # and a custom info.plist to use it
        set(LAUNCH_SCREEN "${PROJECT_SOURCE_DIR}/examples/assets/LaunchScreen.storyboard")
        set(LOGO "${PROJECT_SOURCE_DIR}/examples/assets/logo.png")
        set(INFO_PLIST "${PROJECT_SOURCE_DIR}/examples/assets/info.plist")
        set(ICONS "${PROJECT_SOURCE_DIR}/examples/assets/icon.icns")
        add_executable(${target} MACOSX_BUNDLE ${target_input} ${LAUNCH_SCREEN} ${LOGO} ${ICONS})
        set(RESOURCES ${LAUNCH_SCREEN} ${LOGO} ${ICONS})
        set_target_properties(${target} PROPERTIES RESOURCE "${RESOURCES}"
                                                   MACOSX_BUNDLE_INFO_PLIST ${INFO_PLIST}
                                                   MACOSX_BUNDLE_ICON_FILE icon.icns)
        target_link_libraries(${target} PRIVATE Zancle::Main)
    else()
        add_executable(${target} ${target_input})
    endif()

    # add shared include directory
    target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/examples/include)
    target_link_libraries(${target} PRIVATE ExampleUtils)

    # enable precompiled headers
    if (ZA_ENABLE_PCH)
        message(VERBOSE "enabling PCH for Zancle example '${target}'")
        target_precompile_headers(${target} REUSE_FROM zancle-system)
        target_link_libraries(${target} PRIVATE Threads::Threads)
    endif()

    set_target_warnings(${target})
    set_public_symbols_hidden(${target})

    # Disable GCC's `-Wmissing-field-initializers`, too noisy when using
    # aggregate initialization with many fields
    if(ZA_COMPILER_GCC)
        target_compile_options(${target} PRIVATE -Wno-missing-field-initializers)
    endif()

    # set the debug suffix
    set_target_properties(${target} PROPERTIES DEBUG_POSTFIX -d)

    # set the target's folder (for IDEs that support it, e.g. Visual Studio)
    set_target_properties(${target} PROPERTIES FOLDER "Examples")

    # set the target flags to use the appropriate C++ standard library
    zancle_set_stdlib(${target})

    # set the properties required for debugging
    set_target_properties(${target} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

        XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        XCODE_GENERATE_SCHEME ON)

    # link the target to its Zancle dependencies
    if(THIS_DEPENDS)
        target_link_libraries(${target} PRIVATE ${THIS_DEPENDS})
    endif()

    # set required compile/link options for emscripten and preload resource files
    zancle_apply_emscripten_options(${target})
    if(ZA_OS_EMSCRIPTEN)
        set_target_properties(${target} PROPERTIES SUFFIX ".html")

        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
            # 1. Define a unique path for this target's resources in the build directory
            set(TARGET_RESOURCE_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/staging_${target}")

            # 2. Copy resources to this isolated staging area
            # This prevents target A and target B from overwriting each other in the build folder
            file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${TARGET_RESOURCE_STAGING_DIR})

            # 3. Embed the isolated folder into the WASM virtual filesystem
            # The syntax "physical_path@virtual_path" maps our unique staging folder
            # to the "resources" folder the C++ code expects at runtime.
            target_link_options(${target} PRIVATE
                "SHELL:--embed-file ${TARGET_RESOURCE_STAGING_DIR}/resources@resources"
            )

            # Ensure the target rebuilds if resources change
            set_property(TARGET ${target} APPEND PROPERTY SUBDIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/resources)
        endif()
    endif()

    if(ZA_OS_IOS)
        zancle_set_common_ios_properties(${target})
    endif()

    if(ZA_OS_WINDOWS AND ZA_USE_MESA3D)
        add_dependencies(${target} "install-mesa3d")
    endif()

    # Enable support for UTF-8 characters in source code
    if(ZA_COMPILER_MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
    endif()
endmacro()

# add a new target which is a Zancle test
#
# example: zancle_add_test(test-zancle-network
#                          SOURCES ftp.cpp http.cpp
#                          DEPENDS Zancle::Network
#                          RESOURCES doodle_pop.ogg ding.flac ...)
#
# RESOURCES are bundled into the test app on iOS (MACOSX_PACKAGE_LOCATION
# derived from the target's "test-zancle-<name>" suffix, e.g.
# "test-zancle-audio" -> "Audio") and preloaded into the WASM virtual filesystem
# on Emscripten via a per-target staging directory + `--preload-file`.
# `--preload-file` (rather than `--embed-file`) is used because non-ASCII
# filenames (e.g. ń, snail emoji) break --embed-file's symbol generation.
function(zancle_add_test target)
    cmake_parse_arguments(THIS "" "" "SOURCES;DEPENDS;RESOURCES" ${ARGN})
    if(NOT "${THIS_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Extra unparsed arguments when calling zancle_add_test: ${THIS_UNPARSED_ARGUMENTS}")
    endif()

    # Backwards-compatible aliases (the previous positional signature was
    # zancle_add_test(target SOURCES DEPENDS); keep DEPENDS as a callable
    # symbol below so the rest of the function reads naturally.)
    set(SOURCES "${THIS_SOURCES}")
    set(DEPENDS "${THIS_DEPENDS}")

    # create the target
    add_executable(${target} ${SOURCES})

    # set required compile/link options for emscripten
    zancle_apply_emscripten_options(${target})
    if(ZA_OS_EMSCRIPTEN)
        if (${target} STREQUAL "test-zancle-graphics" OR ${target} STREQUAL "test-zancle-audio")
            set_target_properties(${target} PROPERTIES SUFFIX ".html")
        else()
            set_target_properties(${target} PROPERTIES SUFFIX ".js")
        endif()
    endif()

    # enable precompiled headers
    if (ZA_ENABLE_PCH)
        message(VERBOSE "enabling PCH for Zancle test '${target}'")
        target_precompile_headers(${target} REUSE_FROM zancle-system)
        target_link_libraries(${target} PRIVATE Threads::Threads)
    endif()

    # set the target's folder (for IDEs that support it, e.g. Visual Studio)
    set_target_properties(${target} PROPERTIES FOLDER "Tests")

    # set the target flags to use the appropriate C++ standard library
    zancle_set_stdlib(${target})

    set_target_properties(${target} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} # set the Visual Studio startup path for debugging
        VS_DEBUGGER_COMMAND_ARGUMENTS "-b" # Break into debugger

        XCODE_GENERATE_SCHEME ON # Required to set arguments
        XCODE_SCHEME_ARGUMENTS "-b" # Break into debugger
        XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} # set the Xcode startup path for debugging
    )

    # link the target to its Zancle dependencies
    target_link_libraries(${target} PRIVATE ${DEPENDS} zancle-test-main)

    set_target_warnings(${target})
    set_public_symbols_hidden(${target})

    # If coverage is enabled for MSVC and we are linking statically, use /WHOLEARCHIVE
    # to make sure the linker doesn't discard unused code sections before coverage can be measured
    if(ZA_ENABLE_COVERAGE AND ZA_COMPILER_MSVC AND NOT BUILD_SHARED_LIBS)
        foreach(DEPENDENCY ${DEPENDS})
            target_link_options(${target} PRIVATE $<$<CONFIG:DEBUG>:/WHOLEARCHIVE:$<TARGET_LINKER_FILE:${DEPENDENCY}>>)
        endforeach()
    endif()

    if(ZA_OS_WINDOWS AND ZA_USE_MESA3D)
        add_dependencies(${target} "install-mesa3d")
    endif()

    # Delay test registration when cross compiling to avoid running crosscompiled app on host OS
    if(CMAKE_CROSSCOMPILING)
        set(CMAKE_CATCH_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)

        # When running tests on Android, use a custom shell script to invoke commands using adb shell
        if(ZA_OS_ANDROID)
            set_target_properties(${target} PROPERTIES CROSSCOMPILING_EMULATOR "${PROJECT_BINARY_DIR}/run-in-adb-shell.sh")
        endif()
    endif()

    # Required to actually run the tests
    if(ZA_OS_IOS)
        zancle_set_common_ios_properties(${target})
    endif()

    # Enable support for UTF-8 characters in source code
    if(ZA_COMPILER_MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
    endif()

    # Bundle and (on Emscripten) preload resources.
    if(THIS_RESOURCES AND (ZA_OS_IOS OR ZA_OS_EMSCRIPTEN))
        # Derive the iOS package subdir from the target: test-zancle-audio -> Audio
        string(REGEX REPLACE "^test-zancle-" "" _bundle "${target}")
        string(SUBSTRING "${_bundle}" 0 1 _bundle_first)
        string(TOUPPER  "${_bundle_first}" _bundle_first)
        string(SUBSTRING "${_bundle}" 1 -1 _bundle_rest)
        set(_bundle "${_bundle_first}${_bundle_rest}")

        target_sources(${target} PRIVATE ${THIS_RESOURCES})
        set_source_files_properties(${THIS_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION "${_bundle}")

        if(ZA_OS_EMSCRIPTEN)
            set(_staging "${CMAKE_CURRENT_BINARY_DIR}/staging_${target}")
            foreach(_r IN LISTS THIS_RESOURCES)
                file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${_r}" DESTINATION "${_staging}")
            endforeach()
            target_link_options(${target} PRIVATE "SHELL:--preload-file ${_staging}@/")
        endif()
    endif()

    # Add the test
    if(NOT ZA_OS_EMSCRIPTEN)
        # Wire up LSan suppressions for the dbus / GL driver / Vulkan leaks that
        # surface during graphics-context teardown. The suppressions list lives
        # in `lsan_suppressions.txt` at the project root.
        #
        # The bespoke testing library runs every test case from a single
        # binary entry point; the binary's exit code reports overall pass/fail
        # and per-case diagnostics go to stderr. One ctest entry per target
        # is sufficient (and faster to register than per-case discovery).
        add_test(NAME ${target}
                 COMMAND ${target}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
        set_tests_properties(${target} PROPERTIES
            ENVIRONMENT "LSAN_OPTIONS=suppressions=${PROJECT_SOURCE_DIR}/lsan_suppressions.txt")
    endif()
endfunction()

# Generate a ZancleConfig.cmake file (and associated files) from the targets registered
# in ZA_ADD_LIBRARY_MODULES_PROPERTY (EXPORT parameter of install(TARGETS))
function(zancle_export_targets)
    # CMAKE_CURRENT_LIST_DIR or CMAKE_CURRENT_SOURCE_DIR not usable for files that are to be included like this one
    set(CURRENT_DIR "${PROJECT_SOURCE_DIR}/cmake")

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/ZancleConfigVersion.cmake"
                                     VERSION ${PROJECT_VERSION}
                                     COMPATIBILITY SameMajorVersion)

    if(ZA_BUILD_FRAMEWORKS)
        set(config_package_location "Zancle.framework/Resources/CMake")
    else()
        set(config_package_location ${CMAKE_INSTALL_LIBDIR}/cmake/Zancle)
    endif()
    configure_package_config_file("${CURRENT_DIR}/ZancleConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/ZancleConfig.cmake"
        INSTALL_DESTINATION "${config_package_location}")

    # generate and install export files for all modules that were added to the list in zancle_add_library()
    if(BUILD_SHARED_LIBS)
        set(config_name "Shared")
    else()
        set(config_name "Static")
    endif()
    get_property(ZA_ADD_LIBRARY_MODULES GLOBAL PROPERTY ZA_ADD_LIBRARY_MODULES_PROPERTY)
    foreach(module ${ZA_ADD_LIBRARY_MODULES})
        install(EXPORT Zancle${module}${config_name}Targets
                FILE Zancle${module}${config_name}Targets.cmake
                DESTINATION ${config_package_location})
    endforeach()

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ZancleConfig.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/ZancleConfigVersion.cmake"
            DESTINATION ${config_package_location}
            COMPONENT devel)
endfunction()
