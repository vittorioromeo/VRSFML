# all source files
set(SRC Rockets.cpp)

# define the rockets target
zancle_add_example(rockets GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Graphics Zancle::ImGui ImGui::ImGui)

target_link_libraries(rockets PRIVATE ExampleProfilerEnabled)
