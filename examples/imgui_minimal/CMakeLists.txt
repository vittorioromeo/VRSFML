# all source files
set(SRC ImGuiMinimal.cpp)

# define the imgui_minimal target
zancle_add_example(imgui_minimal GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics Zancle::ImGui ImGui::ImGui)
