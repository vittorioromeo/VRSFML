# all source files
set(SRC Arkanoid.cpp)

# define the arkanoid target
sfml_add_example(arkanoid GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS SFML::Graphics)
