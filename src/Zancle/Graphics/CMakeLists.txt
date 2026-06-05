set(INCROOT ${PROJECT_SOURCE_DIR}/include/Zancle/Graphics)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/Graphics)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# define the zancle-graphics target
zancle_add_library(Graphics
                 SOURCES ${SRC})

# setup dependencies
target_link_libraries(zancle-graphics PUBLIC Zancle::Window)

# stb_image sources
target_include_directories(zancle-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/stb_image")

# glad sources
target_include_directories(zancle-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# qoi sources
target_include_directories(zancle-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/qoi")

# find external libraries
if(ZA_OS_ANDROID)
    target_link_libraries(zancle-graphics PRIVATE z)
elseif(ZA_OS_IOS)
    target_link_libraries(zancle-graphics PRIVATE z bz2)
endif()

if(ZA_USE_SYSTEM_DEPS)
    find_package(Freetype REQUIRED)
else()
    # Scope CPM/option overrides without leaking them to the parent scope.
    block()
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(BUILD_SHARED_LIBS OFF)
        set(SKIP_INSTALL_HEADERS ON)
        set(FT_DISABLE_ZLIB ON)
        set(FT_DISABLE_BZIP2 ON)
        set(FT_DISABLE_PNG ON)
        set(FT_DISABLE_HARFBUZZ ON)
        set(FT_DISABLE_BROTLI ON)
        set(FT_ENABLE_ERROR_STRINGS ON)

        CPMAddPackage(
            NAME freetype
            GITHUB_REPOSITORY freetype/freetype
            GIT_TAG VER-2-14-3
            # patch out parts we don't want of the FreeType CMake configuration
            # - installing pkgconfig files
            PATCH_COMMAND ${CMAKE_COMMAND} -DFREETYPE_DIR=. -P ${CMAKE_CURRENT_LIST_DIR}/../../../tools/freetype/PatchFreetype.cmake
            SYSTEM TRUE
        )

        # block() does not export normal variables, so re-publish the
        # CPM-set freetype_SOURCE_DIR for set_source_files_properties below.
        set(freetype_SOURCE_DIR "${freetype_SOURCE_DIR}" PARENT_SCOPE)
    endblock()

    zancle_tame_thirdparty(UNITY_BUILD TARGETS freetype)

    set_source_files_properties(
        "${freetype_SOURCE_DIR}/src/pfr/pfr.c"
        "${freetype_SOURCE_DIR}/src/smooth/smooth.c"
        "${freetype_SOURCE_DIR}/builds/windows/ftsystem.c"
        "${freetype_SOURCE_DIR}/builds/windows/ftdebug.c"
        TARGET_DIRECTORY freetype
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )

    add_library(Freetype::Freetype ALIAS freetype)
endif()

target_link_libraries(zancle-graphics PRIVATE Freetype::Freetype PUBLIC Zancle::GLUtils)

# add preprocessor symbols
target_compile_definitions(zancle-graphics PRIVATE "STBI_FAILURE_USERMSG")

if(ZA_FATAL_OPENGL_ERRORS)
    target_compile_definitions(zancle-graphics PRIVATE "ZA_FATAL_OPENGL_ERRORS")
endif()

# Image.cpp must be compiled with the -fno-strict-aliasing
# when gcc is used; otherwise saving PNGs may crash in stb_image_write
if(ZA_COMPILER_GCC)
    set_source_files_properties(${SRCROOT}/Image.cpp PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)
endif()

# TODO P1: add support harfbuzz/sheenbidi from upstream Zancle
