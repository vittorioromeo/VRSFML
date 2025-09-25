set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Graphics)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Graphics)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# define the sfml-graphics target
sfml_add_library(Graphics
                 SOURCES ${SRC})

# setup dependencies
target_link_libraries(sfml-graphics PUBLIC SFML::Window)

# stb_image sources
target_include_directories(sfml-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/stb_image")

# glad sources
target_include_directories(sfml-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# qoi sources
target_include_directories(sfml-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/qoi")

# find external libraries
if(SFML_OS_ANDROID)
    target_link_libraries(sfml-graphics PRIVATE z)
elseif(SFML_OS_IOS)
    target_link_libraries(sfml-graphics PRIVATE z bz2)
endif()

if(SFML_USE_SYSTEM_DEPS)
    find_package(Freetype REQUIRED)
else()
    # use an immediately invoked function to scope option variables we have to set
    function(sfml_add_graphics_dependencies)
        # remember whether we are building SFML as a shared library
        if(BUILD_SHARED_LIBS)
            set(SFML_BUILD_SHARED_LIBS ON)
        endif()

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
            GIT_TAG VER-2-13-3
            # patch out parts we don't want of the FreeType CMake configuration
            # - installing pkgconfig files
            PATCH_COMMAND ${CMAKE_COMMAND} -DFREETYPE_DIR=. -P ${CMAKE_CURRENT_LIST_DIR}/../../../tools/freetype/PatchFreetype.cmake
            SYSTEM TRUE
        )

        set_target_properties(freetype PROPERTIES FOLDER "Dependencies")

        target_compile_options(freetype PRIVATE "-w")

        # add flags required for emscripten support (e.g. pthread)
        if(SFML_OS_EMSCRIPTEN)
            target_compile_options(freetype PRIVATE ${SFML_EMSCRIPTEN_TARGET_COMPILE_OPTIONS})
            target_link_options(freetype PRIVATE ${SFML_EMSCRIPTEN_TARGET_LINK_OPTIONS})
        endif()

        # if building SFML as a shared library and linking our dependencies in
        # as static libraries we need to build them with -fPIC
        if(SFML_BUILD_SHARED_LIBS)
            set_target_properties(freetype PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif()

        # build using unity build and exclude files that cause issues
        set_target_properties(freetype PROPERTIES UNITY_BUILD ON UNITY_BUILD_BATCH_SIZE 256)
        set_source_files_properties(
            "${freetype_SOURCE_DIR}/src/pfr/pfr.c"
            "${freetype_SOURCE_DIR}/src/smooth/smooth.c"
            "${freetype_SOURCE_DIR}/builds/windows/ftsystem.c"
            "${freetype_SOURCE_DIR}/builds/windows/ftdebug.c"
            TARGET_DIRECTORY freetype
            PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
        )

        sfml_set_stdlib(freetype)
        add_library(Freetype::Freetype ALIAS freetype)
    endfunction()
    sfml_add_graphics_dependencies()
endif()

target_link_libraries(sfml-graphics PRIVATE Freetype::Freetype PUBLIC SFML::GLUtils)

# add preprocessor symbols
target_compile_definitions(sfml-graphics PRIVATE "STBI_FAILURE_USERMSG")

if(SFML_FATAL_OPENGL_ERRORS)
    target_compile_definitions(sfml-graphics PRIVATE "SFML_FATAL_OPENGL_ERRORS")
endif()

# Image.cpp must be compiled with the -fno-strict-aliasing
# when gcc is used; otherwise saving PNGs may crash in stb_image_write
if(SFML_COMPILER_GCC)
    set_source_files_properties(${SRCROOT}/Image.cpp PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)
endif()
