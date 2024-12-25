# all source files
set(SRC Batching.cpp)

# define the batching target
sfml_add_example(batching GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui
                 RESOURCES_DIR resources)
