# all source files
set(SRC Arkanoid.cpp)

# define the arkanoid target
zancle_add_example(arkanoid GUI_APP
                 SOURCES ${SRC}
                 BUNDLE_RESOURCES ${RESOURCES}
                 DEPENDS Zancle::Graphics)
