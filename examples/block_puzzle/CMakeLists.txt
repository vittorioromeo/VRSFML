# all source files
set(SRC BlockPuzzle.cpp)

# define the arkanoid target
sfml_add_example(block_puzzle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui)
