# all source files
set(SRC ImGuiMultipleWindows.cpp)

# define the imgui_multiple_windows target
zancle_add_example(imgui_multiple_windows GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics Zancle::ImGui ImGui::ImGui)
