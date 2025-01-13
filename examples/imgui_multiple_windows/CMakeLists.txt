# all source files
set(SRC ImGuiMultipleWindows.cpp)

# define the imgui_multiple_windows target
sfml_add_example(imgui_multiple_windows GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)
