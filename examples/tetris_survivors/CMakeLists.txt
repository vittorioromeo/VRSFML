# all source files
set(SRC TetrisSurvivors.cpp)

# define the arkanoid target
zancle_add_example(tetris_survivors GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio Zancle::Graphics Zancle::ImGui ImGui::ImGui)

target_link_libraries(tetris_survivors PRIVATE ExampleProfilerEnabled)
