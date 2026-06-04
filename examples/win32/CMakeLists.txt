# all source files
set(SRC Win32.cpp)

# define the win32 target
zancle_add_example(win32 GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics)
