cmake_minimum_required(VERSION 3.28)
set(CMAKE_POLICY_VERSION_MINIMUM "3.5") # workaround for dependencies

# define a macro that helps defining an option
macro(sfml_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

# these options have to be set before CMake detects/configures the toolchain

# use new MSVC debug information format specification mechanism if available
# we use this mechanism to embed debug information into the object file to allow ccache to cache it
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
endif()

# determine whether to create a debug or release build
sfml_set_option(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")
sfml_set_option(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" STRING "The minimal iOS version that will be able to run the built binaries. Cannot be lower than 13.0")

# project name
project(SFML VERSION 3.1.0)

set(VERSION_IS_RELEASE OFF)

# include the configuration file
include(cmake/Config.cmake)

# we use the paths from the cmake GNUInstallDirs module as defaults
# you can override these if you like
# https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# disable static libraries on Android
if(SFML_OS_ANDROID)
    set(BUILD_SHARED_LIBS ON)
endif()

# suppress annoying warnings on windows
if(SFML_OS_WINDOWS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# avoid transitive includes with libc++
add_definitions(-D_LIBCPP_REMOVE_TRANSITIVE_INCLUDES)

# include CPM to manage external dependencies
include(cmake/CPM.cmake)

# add an option to enable stack traces on assertions
sfml_set_option(SFML_ENABLE_STACK_TRACES $<CONFIG:Debug> BOOL "TRUE to enable lifetime tracking, FALSE to disable it")
if(SFML_ENABLE_STACK_TRACES)
    add_definitions(-DSFML_ENABLE_STACK_TRACES)

    if (NOT SFML_OS_EMSCRIPTEN)
        CPMAddPackage(
            NAME cpptrace
            GITHUB_REPOSITORY vittorioromeo/cpptrace
            GIT_TAG bcd42b677bfd0209add98123e97e6236f81b9c9b
            SYSTEM TRUE
        )

        if(TARGET libzstd_static)
            target_compile_options(libzstd_static PRIVATE "-w")
            set_target_properties(libzstd_static PROPERTIES UNITY_BUILD OFF)
        endif()

        if(TARGET dwarf)
            target_compile_options(dwarf PRIVATE "-w")
            set_target_properties(dwarf PROPERTIES UNITY_BUILD OFF)
        endif()

        target_compile_options(cpptrace-lib PRIVATE "-w")
        set_target_properties(cpptrace-lib PROPERTIES UNITY_BUILD ON)
        set_target_properties(cpptrace-lib PROPERTIES UNITY_BUILD_BATCH_SIZE 64)

        # install(TARGETS cpptrace-lib EXPORT SFMLConfigExport)
        # install(TARGETS dwarf EXPORT SFMLConfigExport)
    endif()
endif()

# add an option to enable lifetime tracking
sfml_set_option(SFML_ENABLE_LIFETIME_TRACKING $<CONFIG:Debug> BOOL "TRUE to enable lifetime tracking, FALSE to disable it")
if(SFML_ENABLE_LIFETIME_TRACKING)
    add_definitions(-DSFML_ENABLE_LIFETIME_TRACKING)
endif()

# option to enable precompiled headers
sfml_set_option(SFML_ENABLE_PCH OFF BOOL "TRUE to enable precompiled headers for SFML builds -- only supported on Windows/Linux and for static library builds")

if(SFML_ENABLE_PCH)
    add_definitions(-DSFML_ENABLE_PCH)
endif()

# add options to select which modules to build
sfml_set_option(SFML_BUILD_GLUTILS ON BOOL "ON to build SFML's GLUtils module.")
sfml_set_option(SFML_BUILD_WINDOW ON BOOL "ON to build SFML's Window module. This setting is ignored, if the graphics module is built.")
sfml_set_option(SFML_BUILD_GRAPHICS ON BOOL "ON to build SFML's Graphics module.")
sfml_set_option(SFML_BUILD_IMGUI ON BOOL "ON to build SFML's ImGui module.")
sfml_set_option(SFML_BUILD_AUDIO ON BOOL "ON to build SFML's Audio module.")
sfml_set_option(SFML_BUILD_NETWORK ON BOOL "ON to build SFML's Network module.")

if(SFML_BUILD_GLUTILS)
    add_definitions(-DSFML_BUILD_GLUTILS)
endif()

if(SFML_BUILD_WINDOW)
    add_definitions(-DSFML_BUILD_WINDOW)
endif()

if(SFML_BUILD_GRAPHICS)
    add_definitions(-DSFML_BUILD_GRAPHICS)
endif()

if(SFML_BUILD_IMGUI)
    add_definitions(-DSFML_BUILD_IMGUI)
endif()

if(SFML_BUILD_AUDIO)
    add_definitions(-DSFML_BUILD_AUDIO)
endif()

if(SFML_BUILD_NETWORK)
    add_definitions(-DSFML_BUILD_NETWORK)
endif()

if(SFML_BUILD_WINDOW)
    # add an option for choosing the OpenGL implementation
    sfml_set_option(SFML_OPENGL_ES ${OPENGL_ES} BOOL "ON to use an OpenGL ES implementation, OFF to use a desktop OpenGL implementation")

    # add an option for choosing whether to use the DRM windowing backend
    if(SFML_OS_LINUX)
        sfml_set_option(SFML_USE_DRM OFF BOOL "ON to use DRM windowing backend")
    endif()
endif()

# macOS specific options
if(SFML_OS_MACOS OR SFML_OS_IOS)
    # add an option to build frameworks instead of dylibs (release only)
    sfml_set_option(SFML_BUILD_FRAMEWORKS OFF BOOL "ON to build SFML as frameworks libraries (release only), OFF to build according to BUILD_SHARED_LIBS")

    # add an option to automatically install Xcode templates
    sfml_set_option(SFML_INSTALL_XCODE_TEMPLATES OFF BOOL "ON to automatically install the Xcode templates, OFF to do nothing about it. The templates are compatible with Xcode 4 and 5.")
endif()

# Android options
if(SFML_OS_ANDROID)
    sfml_set_option(SFML_ANDROID_USE_SUSPEND_AWARE_CLOCK OFF BOOL "ON to use an sf::Clock implementation which takes system sleep time into account (keeps advancing during suspension), OFF to default to another available monotonic clock")
    if(SFML_ANDROID_USE_SUSPEND_AWARE_CLOCK)
        add_definitions(-DSFML_ANDROID_USE_SUSPEND_AWARE_CLOCK)
    endif()

    # avoid missing libraries when building SFML for Android with NDK r19c and later
    list(PREPEND CMAKE_FIND_ROOT_PATH "${PROJECT_SOURCE_DIR}")

    # install everything in $NDK/sources/ because this path is appended by the NDK (convenient)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_ANDROID_NDK}/sources/third_party/sfml)

    # we install libs in a subdirectory named after the ABI
    set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/${CMAKE_ANDROID_ARCH_ABI}")
endif()

# Install directories
# For miscellaneous files
if(SFML_OS_WINDOWS OR SFML_OS_IOS)
    set(DEFAULT_INSTALL_MISC_DIR .)
elseif(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    set(DEFAULT_INSTALL_MISC_DIR share/SFML)
elseif(SFML_OS_MACOS)
    set(DEFAULT_INSTALL_MISC_DIR /usr/local/share/SFML)
elseif(SFML_OS_ANDROID)
    set(DEFAULT_INSTALL_MISC_DIR ${CMAKE_ANDROID_NDK}/sources/third_party/sfml)
endif()

# force building sfml-window, if sfml-graphics module is built
if(SFML_BUILD_GRAPHICS AND NOT SFML_BUILD_WINDOW)
    message(WARNING "You're trying to build SFML's Graphics module without the Window module. Forcing building of the Window module as a dependency.")
    set(SFML_BUILD_WINDOW ON)
endif()

# force building sfml-graphics, if sfml-imgui module is built
if(SFML_BUILD_IMGUI AND NOT SFML_BUILD_GRAPHICS)
    message(WARNING "You're trying to build SFML's ImGui module without the Graphics module. Forcing building of the Graphics module as a dependency.")
    set(SFML_BUILD_GRAPHICS TRUE)
endif()

# use bundled dependencies by default on these systems and system dependencies by default on all others (maintains old behavior)
if(SFML_OS_WINDOWS OR SFML_OS_LINUX OR SFML_OS_MACOS OR SFML_OS_IOS OR SFML_OS_ANDROID OR SFML_OS_EMSCRIPTEN)
    set(SFML_USE_SYSTEM_DEPS_DEFAULT OFF)
else()
    set(SFML_USE_SYSTEM_DEPS_DEFAULT ON)
endif()

sfml_set_option(SFML_USE_SYSTEM_DEPS ${SFML_USE_SYSTEM_DEPS_DEFAULT} BOOL "ON to use system dependencies, OFF to use the bundled ones.")
if(SFML_USE_SYSTEM_DEPS AND SFML_INSTALL_XCODE_TEMPLATES)
    message(FATAL_ERROR "XCode templates installation cannot be used with the SFML_USE_SYSTEM_DEPS option (the bundled frameworks are required.)")
endif()

if(SFML_COMPILER_MSVC)
    # add an option to choose whether PDB debug symbols should be generated (defaults to true when possible)
    sfml_set_option(SFML_GENERATE_PDB ON BOOL "True to generate PDB debug symbols, OFF otherwise.")

    # if building using a compiler launcher, embed the MSVC debugging information to allow for caching
    if(CMAKE_CXX_COMPILER_LAUNCHER)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
    endif()
endif()

# define SFML_OPENGL_ES if needed
if(SFML_OPENGL_ES)
    add_definitions(-DSFML_OPENGL_ES)
endif()

# define an option for choosing between static and dynamic C runtime (Windows only)
if(SFML_OS_WINDOWS)
    sfml_set_option(SFML_USE_STATIC_STD_LIBS OFF BOOL "ON to statically link to the standard libraries, OFF to use them as DLLs")

    # the following combination of flags is not valid
    if(BUILD_SHARED_LIBS AND SFML_USE_STATIC_STD_LIBS)
        message(FATAL_ERROR "BUILD_SHARED_LIBS and SFML_USE_STATIC_STD_LIBS cannot be used together")
    endif()

    sfml_set_option(SFML_USE_MESA3D OFF BOOL "ON to use the Mesa 3D graphics library for rendering, OFF to use the system provided library for rendering")

    include(cmake/Mesa3D.cmake)
endif()

# setup macOS stuff
if(SFML_OS_MACOS)
    # SFML_BUILD_FRAMEWORKS needs two things:
    # first, it's available only for release
    #    (because cmake currently doesn't allow specifying a custom framework name so XXX-d is not possible)
    # secondly, it works only with BUILD_SHARED_LIBS enabled
    if(SFML_BUILD_FRAMEWORKS)
        # requirement #1
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
            message(FATAL_ERROR "CMAKE_BUILD_TYPE should be \"Release\" when SFML_BUILD_FRAMEWORKS is ON")
            return()
        endif()

        # requirement #2
        if(NOT BUILD_SHARED_LIBS)
            message(FATAL_ERROR "BUILD_SHARED_LIBS should be ON when SFML_BUILD_FRAMEWORKS is ON")
            return()
        endif()
    endif()

    # configure Xcode templates
    set(XCODE_TEMPLATES_ARCH "\$(NATIVE_ARCH_ACTUAL)")

    if(NOT SFML_COMPILER_CLANG)
        message(FATAL_ERROR "Clang is the only supported compiler on macOS")
    endif()
endif()

option(SFML_ENABLE_SANITIZERS "Enable sanitizers" OFF)
if(SFML_ENABLE_SANITIZERS)
    string(APPEND CMAKE_CXX_FLAGS " -fno-omit-frame-pointer -fno-sanitize-recover=all -fsanitize=undefined")
endif()

if(SFML_BUILD_EXAMPLES)
    add_definitions(-DSFML_BUILD_EXAMPLES)
endif()

if(SFML_BUILD_TEST_SUITE)
    # this definition is used in the PCH file to speed up compilation of tests
    # `add_definitions` must be called before `add_subdirectory`
    add_definitions(-DSFML_BUILD_TEST_SUITE)
    # add_definitions(-DDOCTEST_CONFIG_NO_INCLUDE_IOSTREAM=1)
    add_definitions(-DDOCTEST_CONFIG_SUPER_FAST_ASSERTS)
endif()

option(SFML_ENABLE_STDLIB_ASSERTIONS "Enable standard library assertions" OFF)
if(SFML_ENABLE_STDLIB_ASSERTIONS)
    # intentionally using `add_compile_definitions` here to propagate defines to subdirectories
    add_compile_definitions(_GLIBCXX_ASSERTIONS=1)                                   # see https://gcc.gnu.org/wiki/LibstdcxxDebugMode

    # Xcode has a property to control this - adding the define ourselves causes multiple definitions
    if(XCODE)
        set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_STANDARD_LIBRARY_HARDENING "Yes (extensive)")
    else()
        add_compile_definitions(_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_EXTENSIVE) # see https://libcxx.llvm.org/Hardening.html
    endif()
endif()

option(SFML_ENABLE_HARDENING_FLAGS "Enable hardening flags" OFF)
if(SFML_ENABLE_HARDENING_FLAGS AND NOT MSVC)
    add_compile_options(
        -ftrivial-auto-var-init=pattern
        -fstack-protector-strong
        # -fstack-clash-protection (doesn't work on libc++)
        -fstrict-flex-arrays=3
        # -fno-delete-null-pointer-checks (broken on GCC)
        -ftrapv
    )
endif()

# set the output directory for SFML DLLs and executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# enable project folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# on Linux and BSD-like OS, install pkg-config files by default
set(SFML_INSTALL_PKGCONFIG_DEFAULT OFF)

if(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    set(SFML_INSTALL_PKGCONFIG_DEFAULT ON)
endif()

sfml_set_option(SFML_INSTALL_PKGCONFIG_FILES ${SFML_INSTALL_PKGCONFIG_DEFAULT} BOOL "ON to automatically install pkg-config files so other projects can find SFML")

if(SFML_INSTALL_PKGCONFIG_FILES)
    # account for CMAKE_INSTALL_LIBDIR potentially being an absolute path
    file(RELATIVE_PATH SFML_RELATIVE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX} ${CMAKE_INSTALL_FULL_LIBDIR})

    # set pkgconfig install directory
    # this could be e.g. macports on mac or msys2 on windows etc.
    set(SFML_PKGCONFIG_DIR "${SFML_RELATIVE_INSTALL_LIBDIR}/pkgconfig")

    if(SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
        set(SFML_PKGCONFIG_DIR "libdata/pkgconfig")
    endif()

    sfml_set_option(SFML_PKGCONFIG_INSTALL_DIR "${SFML_PKGCONFIG_DIR}" PATH "Install directory for SFML's pkg-config .pc files")

    if(SFML_BUILD_AUDIO AND SFML_BUILD_NETWORK AND SFML_BUILD_GRAPHICS AND SFML_BUILD_WINDOW)
        configure_file(
            "tools/pkg-config/sfml-all.pc.in"
            "tools/pkg-config/sfml-all.pc"
            @ONLY)
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tools/pkg-config/sfml-all.pc"
            DESTINATION "${SFML_PKGCONFIG_INSTALL_DIR}")
    endif()
endif()

if(SFML_ENABLE_PCH AND BUILD_SHARED_LIBS)
    message(FATAL_ERROR "Precompiled headers are currently not supported for shared library builds")
endif()

if(SFML_ENABLE_PCH AND SFML_OS_MACOS)
    message(FATAL_ERROR "Precompiled headers are currently not supported in macOS builds")
endif()

# add the subdirectories
add_subdirectory(src/SFML)

# setup the install rules
if(NOT SFML_BUILD_FRAMEWORKS)
    install(DIRECTORY include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            COMPONENT devel
            FILES_MATCHING PATTERN "*.hpp" PATTERN "*.inl")
    install(DIRECTORY cmake/Modules/
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/SFML
            COMPONENT devel)

    if(SFML_GENERATE_PDB)
        install(DIRECTORY ${PROJECT_BINARY_DIR}/lib/
                DESTINATION ${CMAKE_INSTALL_LIBDIR}
                COMPONENT devel
                FILES_MATCHING PATTERN "*.pdb")
    endif()
else()
    # find only "root" headers
    file(GLOB SFML_HEADERS RELATIVE ${PROJECT_SOURCE_DIR} "include/SFML/*")

    # Because we use generator expressions in the post build scripts we need to
    # suppress the generation of "EFFECTIVE_PLATFORM_NAME" as it will fail
    set_property(GLOBAL PROPERTY XCODE_EMIT_EFFECTIVE_PLATFORM_NAME OFF)

    # in fact we have to fool cmake to copy all the headers in subdirectories
    # to do that we have to add the "root" headers to the PUBLIC_HEADER
    # then we can run a post script to copy the remaining headers

    # we need a dummy file in order to compile the framework
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
                       COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
                       VERBATIM)

    set(SFML_SOURCES ${SFML_HEADERS})
    list(APPEND SFML_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)

    # create SFML.framework
    add_library(SFML ${SFML_SOURCES})

    # enable C++20 support
    target_compile_features(SFML PUBLIC cxx_std_20)

    # set the target flags to use the appropriate C++ standard library
    sfml_set_stdlib(SFML)

    # edit target properties
    set_target_properties(SFML PROPERTIES
                          FRAMEWORK ON
                          FRAMEWORK_VERSION ${PROJECT_VERSION}
                          MACOSX_FRAMEWORK_IDENTIFIER org.sfml-dev.SFML
                          MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${PROJECT_VERSION}
                          MACOSX_FRAMEWORK_BUNDLE_VERSION ${PROJECT_VERSION}
                          PUBLIC_HEADER "${SFML_HEADERS}")

    # add the non-optional SFML headers
    add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                       ${PROJECT_SOURCE_DIR}/include/SFML/Config.hpp
                       ${PROJECT_SOURCE_DIR}/include/SFML/OpenGL.hpp
                       ${PROJECT_SOURCE_DIR}/include/SFML/GpuPreference.hpp
                       ${PROJECT_SOURCE_DIR}/include/SFML/Main.hpp
                       ${PROJECT_SOURCE_DIR}/include/SFML/System
                       $<TARGET_FILE_DIR:SFML>/Headers
                       VERBATIM)

    # add glutils module headers if enabled
    if(SFML_BUILD_GLUTILS)
        add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                           ${PROJECT_SOURCE_DIR}/include/SFML/GLUtils
                           $<TARGET_FILE_DIR:SFML>/Headers
                           VERBATIM)
    endif()

    # add window module headers if enabled
    if(SFML_BUILD_WINDOW)
        add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                           ${PROJECT_SOURCE_DIR}/include/SFML/Window
                           $<TARGET_FILE_DIR:SFML>/Headers
                           VERBATIM)
    endif()

    # add network module headers if enabled
    if(SFML_BUILD_NETWORK)
        add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                           ${PROJECT_SOURCE_DIR}/include/SFML/Network
                           $<TARGET_FILE_DIR:SFML>/Headers
                           VERBATIM)
    endif()

     # add graphics module headers if enabled
    if(SFML_BUILD_GRAPHICS)
        add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                           ${PROJECT_SOURCE_DIR}/include/SFML/Graphics
                           $<TARGET_FILE_DIR:SFML>/Headers
                           VERBATIM)
    endif()

    # add imgui module headers if enabled
    if(SFML_BUILD_IMGUI)
        add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                           ${PROJECT_SOURCE_DIR}/include/SFML/ImGui
                           $<TARGET_FILE_DIR:SFML>/Headers
                           VERBATIM)
    endif()

     # add audio module headers if enabled
    if(SFML_BUILD_AUDIO)
        add_custom_command(TARGET SFML POST_BUILD COMMAND cp -r
                           ${PROJECT_SOURCE_DIR}/include/SFML/Audio
                           $<TARGET_FILE_DIR:SFML>/Headers
                           VERBATIM)
    endif()

    # adapt install directory to allow distributing dylibs/frameworks in user's frameworks/application bundle
    # NOTE: it's not required to link against SFML.framework
    set_target_properties(SFML PROPERTIES INSTALL_NAME_DIR "@rpath")
    if(NOT CMAKE_SKIP_BUILD_RPATH)
        set_target_properties(SFML PROPERTIES BUILD_WITH_INSTALL_NAME_DIR ON)
    endif()

    # install rule
    install(TARGETS SFML
            FRAMEWORK DESTINATION "."
            COMPONENT devel)
endif()

install(FILES license.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES readme.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

# install the Xcode templates if requested
if(SFML_INSTALL_XCODE_TEMPLATES)
    # configure the templates plist files
    foreach(TEMPLATE "SFML Compiler" "SFML App")
        configure_file(
                    "tools/xcode/templates/SFML/${TEMPLATE}.xctemplate/TemplateInfo.plist.in"
                    "${CMAKE_CURRENT_BINARY_DIR}/tools/xcode/templates/SFML/${TEMPLATE}.xctemplate/TemplateInfo.plist"
                    @ONLY)
    endforeach()
    install(DIRECTORY "tools/xcode/templates/SFML" "${CMAKE_CURRENT_BINARY_DIR}/tools/xcode/templates/SFML"
            DESTINATION /Library/Developer/Xcode/Templates
            PATTERN "*.in" EXCLUDE
            PATTERN ".DS_Store" EXCLUDE)
endif()

sfml_export_targets()

# configure extras by default when building SFML directly, otherwise hide them
sfml_set_option(SFML_CONFIGURE_EXTRAS ${PROJECT_IS_TOP_LEVEL} BOOL "ON to configure extras, OFF to ignore them")

if(NOT SFML_CONFIGURE_EXTRAS)
    return()
endif()

set(CPACK_PACKAGE_NAME_SUMMARY "Simple and Fast Multimedia Library")
set(CPACK_PACKAGE_VENDOR "SFML Team")
set(CPACK_PACKAGE_FILE_NAME "SFML-${PROJECT_VERSION}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}-${CMAKE_BUILD_TYPE}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SFML ${PROJECT_VERSION}")
set(CPACK_MONOLITHIC_INSTALL ON)

# NSIS configurations
set(CPACK_NSIS_DISPLAY_NAME "SFML ${PROJECT_VERSION} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION})")
set(CPACK_NSIS_CONTACT "team@sfml-dev.org")
set(NSIS_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/nsis/")
string(REGEX REPLACE "/" "\\\\\\\\" NSIS_IMAGE_PATH ${NSIS_IMAGE_PATH})
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "!define MUI_WELCOMEFINISHPAGE_BITMAP \\\"${NSIS_IMAGE_PATH}sidebar.bmp\\\"\n!define MUI_HEADERIMAGE_BITMAP \\\"${NSIS_IMAGE_PATH}header.bmp\\\"\n!define MUI_ICON \\\"${NSIS_IMAGE_PATH}sfml.ico\\\"")

include(CPack)

# add an option for building the API documentation
sfml_set_option(SFML_BUILD_DOC OFF BOOL "ON to generate the API documentation, OFF to ignore it")
if(SFML_BUILD_DOC)
    add_subdirectory(doc)
endif()

# add an option for building the examples
sfml_set_option(SFML_BUILD_EXAMPLES OFF BOOL "ON to build the SFML examples, OFF to ignore them")
if(SFML_BUILD_EXAMPLES AND NOT SFML_OS_ANDROID)
    add_subdirectory(examples)
endif()

# add an option for building the test suite
sfml_set_option(SFML_BUILD_TEST_SUITE OFF BOOL "ON to build the SFML test suite, OFF to ignore it")

# add an option for enabling coverage reporting
sfml_set_option(SFML_ENABLE_COVERAGE OFF BOOL "ON to enable coverage reporting, OFF to ignore it")

if(SFML_BUILD_TEST_SUITE)
    if(SFML_BUILD_WINDOW AND SFML_BUILD_GRAPHICS AND SFML_BUILD_IMGUI AND SFML_BUILD_NETWORK AND SFML_BUILD_AUDIO)
        enable_testing()
        add_subdirectory(test)
    else()
        message(WARNING "Cannot build unit testing unless all modules are enabled")
    endif()
endif()

# add an option for making OpenGL errors fatal
sfml_set_option(SFML_FATAL_OPENGL_ERRORS OFF BOOL "ON to make SFML OpenGL errors fatal, OFF to simply warn about them")

sfml_set_option(CLANG_FORMAT_EXECUTABLE clang-format STRING "Override clang-format executable, requires version 17")
add_custom_target(format
    COMMAND ${CMAKE_COMMAND} -DCLANG_FORMAT_EXECUTABLE=${CLANG_FORMAT_EXECUTABLE} -P ./cmake/Format.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} VERBATIM)

sfml_set_option(CLANG_TIDY_EXECUTABLE clang-tidy STRING "Override clang-tidy executable, requires minimum version 14")
add_custom_target(tidy
    COMMAND ${CMAKE_COMMAND} -DCLANG_TIDY_EXECUTABLE=${CLANG_TIDY_EXECUTABLE} -DPROJECT_BINARY_DIR=${PROJECT_BINARY_DIR} -P ./cmake/Tidy.cmake
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} VERBATIM)
