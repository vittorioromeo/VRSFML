file(GLOB GRAPHICS_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-graphics
    SOURCES ${GRAPHICS_SRC}
    DEPENDS Zancle::Graphics
    BUNDLE_NAME Graphics
    RESOURCES
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
        tuffy.ttf)

# Some tests directly instantiate GL utility types whose public headers
# transitively include private headers from `src/Zancle/GLUtils/` and the
# bundled `glad` sources.
target_include_directories(test-zancle-graphics PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(test-zancle-graphics PRIVATE zancle-glad-headers)

if(ZA_RUN_DISPLAY_TESTS)
    target_compile_definitions(test-zancle-graphics PRIVATE ZA_RUN_DISPLAY_TESTS)
endif()
