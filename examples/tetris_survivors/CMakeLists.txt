# all source files
set(SRC TetrisSurvivors.cpp)

# define the arkanoid target
sfml_add_example(tetris_survivors GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui)
