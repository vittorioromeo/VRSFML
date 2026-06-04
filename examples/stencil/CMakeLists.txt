# all source files
set(SRC Stencil.cpp)

# define the stencil target
zancle_add_example(stencil GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics)
