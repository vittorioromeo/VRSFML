# all source files
set(SRC Showcase.cpp)

# define the arkanoid target
sfml_add_example(showcase GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui)
