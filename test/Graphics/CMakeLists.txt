file(GLOB GRAPHICS_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-graphics "${GRAPHICS_SRC}" Zancle::Graphics)

# Some tests directly instantiate GL utility types whose public headers
# transitively include private headers from `src/Zancle/GLUtils/` and the
# bundled `glad` sources.
target_include_directories(test-zancle-graphics PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_include_directories(test-zancle-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

if(ZA_RUN_DISPLAY_TESTS)
    target_compile_definitions(test-zancle-graphics PRIVATE ZA_RUN_DISPLAY_TESTS)
endif()

if (ZA_OS_EMSCRIPTEN OR (XCODE AND ZA_OS_IOS))
    set(GRAPHICS_RESOURCES
        invalid_shader.vert
        zancle-logo-big.bmp
        zancle-logo-big.gif
        zancle-logo-big.jpg
        zancle-logo-big.png
        zancle-logo-big.psd
        zancle-logo-big.qoi
        shader-🐌.vert
        shader-ń.vert
        shader.frag
        shader.geom
        shader.vert
        tuffy-🐌.ttf
        tuffy-ń.ttf
        tuffy.ttf
    )

    target_sources(test-zancle-graphics PRIVATE ${GRAPHICS_RESOURCES})
    set_source_files_properties(${GRAPHICS_RESOURCES} MACOSX_PACKAGE_LOCATION "Graphics")

    if (ZA_OS_EMSCRIPTEN)
        # Use --preload-file instead of --embed-file to support non-ASCII filenames.
        # --embed-file generates assembly symbols derived from filenames, which the
        # assembler rejects for non-ASCII characters (e.g. ń, 🐌). --preload-file
        # stores filenames as JS strings in a .data bundle, avoiding the issue.
        set(GRAPHICS_RESOURCE_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/staging_test-zancle-graphics")

        foreach(RESOURCE ${GRAPHICS_RESOURCES})
            file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE}" DESTINATION "${GRAPHICS_RESOURCE_STAGING_DIR}")
        endforeach()

        target_link_options(test-zancle-graphics PRIVATE
            "SHELL:--preload-file ${GRAPHICS_RESOURCE_STAGING_DIR}@/")
    endif()
endif()
