# all source files
set(SRC Joystick.cpp)

# define the joystick target
zancle_add_example(joystick GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics)
