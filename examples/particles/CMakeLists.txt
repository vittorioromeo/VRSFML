# all source files
set(SRC Particles.cpp)

# define the particles target
zancle_add_example(particles GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics Zancle::ImGui ImGui::ImGui)
