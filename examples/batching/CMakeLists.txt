# all source files
set(SRC Batching.cpp)

# define the batching target
zancle_add_example(batching GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics Zancle::ImGui ImGui::ImGui)
