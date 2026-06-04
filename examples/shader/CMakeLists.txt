# all source files
set(SRC Shader.cpp)

# define the shader target
zancle_add_example(shader GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics)
