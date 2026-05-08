# all source files
set(SRC PixelPerfect.cpp)

# define the pixel_perfect target
sfml_add_example(pixel_perfect GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics)
