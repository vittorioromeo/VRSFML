# all source files
set(SRC Rockets.cpp)

# define the rockets target
sfml_add_example(rockets GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)

find_package(Boost 1.70 REQUIRED COMPONENTS pfr)
target_link_libraries(rockets PRIVATE Boost::pfr)
