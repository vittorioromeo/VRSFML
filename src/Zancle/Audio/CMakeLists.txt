set(INCROOT ${PROJECT_SOURCE_DIR}/include/Zancle/Audio)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/Audio)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# Ensure certain files are compiled as Objective-C++
# See: https://miniaud.io/docs/manual/index.html#Building
if(ZA_OS_IOS)
    enable_language(OBJCXX)
    set_source_files_properties(${SRCROOT}/Miniaudio.cpp PROPERTIES LANGUAGE OBJCXX)
endif()

# find external libraries
if(ZA_USE_SYSTEM_DEPS)
    find_package(Vorbis REQUIRED)
    find_package(FLAC REQUIRED)
else()
    # Scope CPM/option overrides without leaking them to the parent scope.
    block()
        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
        set(BUILD_SHARED_LIBS OFF)
        set(BUILD_CXXLIBS OFF)
        set(BUILD_PROGRAMS OFF)
        set(BUILD_EXAMPLES OFF)
        set(BUILD_TESTING OFF)
        set(BUILD_UTILS OFF)
        set(BUILD_DOCS OFF)
        set(INSTALL_MANPAGES OFF)
        set(INSTALL_DOCS OFF)
        set(INSTALL_PKG_CONFIG_MODULE OFF)
        set(INSTALL_PKGCONFIG_MODULES OFF)
        set(WITH_FORTIFY_SOURCE OFF)
        set(WITH_STACK_PROTECTOR OFF)
        set(WITH_AVX OFF) # LLVM/Clang on Windows has issues with AVX2

        CPMAddPackage(
            NAME ogg
            GITHUB_REPOSITORY xiph/ogg
            GIT_TAG v1.3.6
            SYSTEM TRUE
            # patch CMAKE_DEBUG_POSTFIX into the Ogg CMake configuration
            PATCH_COMMAND ${CMAKE_COMMAND} -DOGG_DIR=.  -P ${CMAKE_CURRENT_LIST_DIR}/../../../tools/ogg/PatchOgg.cmake
        )

        CPMAddPackage(
            NAME flac
            GITHUB_REPOSITORY xiph/flac
            GIT_TAG 1.5.0
            SYSTEM TRUE
            # patch out the annoying parts of the FLAC CMake configuration:
            # - adding unnecessary libraries that aren't even used
            # - installing headers & pkgconfig files
            # - add CMAKE_DEBUG_POSTFIX
            PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/../../../tools/flac/PatchFLAC.cmake
        )

        CPMAddPackage(
            NAME vorbis
            GITHUB_REPOSITORY xiph/vorbis
            GIT_TAG v1.3.7
            SYSTEM TRUE
            # patch out the annoying parts of the Vorbis CMake configuration:
            # - Vorbis doesn't check if the Ogg::ogg target exists before calling find_package
            # - installing headers & pkgconfig files
            # - add CMAKE_DEBUG_POSTFIX
            PATCH_COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/../../../tools/vorbis/PatchVorbis.cmake
        )

        # block() does not export normal variables, so re-publish the CPM-set
        # *_SOURCE_DIR paths we need for set_source_files_properties below.
        set(flac_SOURCE_DIR    "${flac_SOURCE_DIR}"    PARENT_SCOPE)
        set(vorbis_SOURCE_DIR  "${vorbis_SOURCE_DIR}"  PARENT_SCOPE)
    endblock()

    zancle_tame_thirdparty(UNITY_BUILD TARGETS ogg FLAC vorbis vorbisenc vorbisfile)

    # files that fail unity build in FLAC
    set_source_files_properties(
        "${flac_SOURCE_DIR}/src/libFLAC/fixed.c"
        "${flac_SOURCE_DIR}/src/libFLAC/format.c"
        "${flac_SOURCE_DIR}/src/libFLAC/fixed_intrin_sse2.c"
        "${flac_SOURCE_DIR}/src/libFLAC/fixed_intrin_ssse3.c"
        "${flac_SOURCE_DIR}/src/libFLAC/fixed_intrin_sse42.c"
        "${flac_SOURCE_DIR}/src/libFLAC/lpc.c"
        "${flac_SOURCE_DIR}/src/libFLAC/metadata_object.c"
        "${flac_SOURCE_DIR}/src/libFLAC/stream_decoder.c"
        "${flac_SOURCE_DIR}/src/libFLAC/stream_encoder.c"
        "${flac_SOURCE_DIR}/src/libFLAC/window.c"
        TARGET_DIRECTORY FLAC
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )
    set_source_files_properties(
        "${vorbis_SOURCE_DIR}/lib/floor1.c"
        "${vorbis_SOURCE_DIR}/lib/sharedbook.c"
        "${vorbis_SOURCE_DIR}/lib/vorbisenc.c"
        TARGET_DIRECTORY vorbis
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )
    set_source_files_properties(
        "${vorbis_SOURCE_DIR}/lib/vorbisenc.c"
        TARGET_DIRECTORY vorbisenc
        PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )

    if(TARGET benchmark_residual)
        set_target_properties(benchmark_residual PROPERTIES UNITY_BUILD OFF)
    endif()

    # define NDEBUG when building FLAC to suppress console debug output
    target_compile_definitions(FLAC PRIVATE NDEBUG)

    # _FILE_OFFSET_BITS=64 is not supported with older (<24) Android API levels
    if(ZA_OS_ANDROID)
        target_compile_definitions(FLAC PRIVATE _FILE_OFFSET_BITS=32)
    endif()

    # aliases were introduced only after 1.3.7 was released
    add_library(Vorbis::vorbis ALIAS vorbis)
    add_library(Vorbis::vorbisenc ALIAS vorbisenc)
    add_library(Vorbis::vorbisfile ALIAS vorbisfile)
endif()

find_package(Threads REQUIRED)

# define the zancle-audio target
zancle_add_library(Audio
                 SOURCES ${SRC}
                 DEPENDENCIES "Dependencies.cmake.in")

# avoids warnings in vorbisfile.h
target_compile_definitions(zancle-audio PRIVATE OV_EXCLUDE_STATIC_CALLBACKS FLAC__NO_DLL)

# disable miniaudio features we do not use
target_compile_definitions(zancle-audio PRIVATE MA_NO_MP3 MA_NO_FLAC MA_NO_ENCODING MA_NO_RESOURCE_MANAGER MA_NO_GENERATION)

# use standard fixed-width integer types
target_compile_definitions(zancle-audio PRIVATE MA_USE_STDINT)

# detect the endianness as required by Ogg
target_compile_definitions(zancle-audio PRIVATE ZA_IS_BIG_ENDIAN=$<STREQUAL:${CMAKE_CXX_BYTE_ORDER},BIG_ENDIAN>)

# setup dependencies
target_link_libraries(zancle-audio
                      PUBLIC Zancle::System
                      PRIVATE Vorbis::vorbis Vorbis::vorbisfile Vorbis::vorbisenc FLAC::FLAC Threads::Threads)

if(ZA_OS_IOS)
    target_link_libraries(zancle-audio PRIVATE "-framework Foundation" "-framework CoreFoundation" "-framework CoreAudio" "-framework AudioToolbox" "-framework AVFoundation")
endif()

# miniaudio sources
target_include_directories(zancle-audio SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/miniaudio")

# needed because PCH is defined in the System module
if(ZA_ENABLE_PCH)
    # TODO P2: restore, needs to be fixed when Zancle is used as a dependency
    # target_include_directories(zancle-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/miniaudio")
endif()

# minimp3 sources
target_include_directories(zancle-audio SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/minimp3")

if(ZA_OS_ANDROID)
    target_link_libraries(zancle-audio PRIVATE android OpenSLES)
endif()

if(ZA_OS_LINUX)
    target_link_libraries(zancle-audio PRIVATE dl)
endif()
