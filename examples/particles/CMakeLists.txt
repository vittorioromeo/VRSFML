# all source files
set(SRC Particles.cpp)

# define the particles target
sfml_add_example(particles GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)
