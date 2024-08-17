# all source files
set(SRC ImGuiMinimal.cpp)

# define the imgui_minimal target
sfml_add_example(imgui_minimal GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui
                 RESOURCES_DIR resources)
