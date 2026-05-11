# all source files
set(SRC Rockets.cpp)

# define the rockets target
sfml_add_example(rockets GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)

target_link_libraries(rockets PRIVATE ExampleProfilerEnabled)
