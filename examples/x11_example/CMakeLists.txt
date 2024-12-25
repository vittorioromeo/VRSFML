find_package(X11 REQUIRED)

# all source files
set(SRC X11Example.cpp)

# define the X11 target
sfml_add_example(x11_example GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Window X11::X11)

# external dependency headers
target_include_directories(x11_example SYSTEM PRIVATE ${PROJECT_SOURCE_DIR}/examples/include)
