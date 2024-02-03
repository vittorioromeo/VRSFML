# all source files
set(SRC FontLifetime.cpp)

if(SFML_OS_IOS)
    set(RESOURCES
        resources/tuffy.ttf)
    set_source_files_properties(${RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

# define the font_lifetime target
sfml_add_example(font_lifetime GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS SFML::Graphics
                 RESOURCES_DIR resources)
