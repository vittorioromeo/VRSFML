file(GLOB GRAPHICS_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
sfml_add_test(test-sfml-graphics "${GRAPHICS_SRC}" SFML::Graphics)

if(SFML_RUN_DISPLAY_TESTS)
    target_compile_definitions(test-sfml-graphics PRIVATE SFML_RUN_DISPLAY_TESTS)
endif()

if (SFML_OS_EMSCRIPTEN OR (XCODE AND SFML_OS_IOS))
    set(GRAPHICS_RESOURCES
        invalid_shader.vert
        sfml-logo-big.bmp
        sfml-logo-big.gif
        sfml-logo-big.jpg
        sfml-logo-big.png
        sfml-logo-big.psd
        shader-🐌.vert
        shader-ń.vert
        shader.frag
        shader.geom
        shader.vert
        tuffy-🐌.ttf
        tuffy-ń.ttf
        tuffy.ttf
    )

    target_sources(test-sfml-graphics PRIVATE ${GRAPHICS_RESOURCES})
    set_source_files_properties(${GRAPHICS_RESOURCES} MACOSX_PACKAGE_LOCATION "Graphics")

    if (SFML_OS_EMSCRIPTEN)
        foreach(RESOURCE ${GRAPHICS_TEST_RESOURCES})
            target_link_options(test-sfml-graphics PRIVATE "SHELL:--embed-file ${RESOURCE}")
        endforeach()
    endif()
endif()
