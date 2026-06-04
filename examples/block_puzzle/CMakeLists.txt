# all source files
set(SRC BlockPuzzle.cpp)

# define the arkanoid target
zancle_add_example(block_puzzle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio Zancle::Graphics Zancle::ImGui ImGui::ImGui)

target_link_libraries(block_puzzle PRIVATE ExampleProfilerEnabled)
