# all source files
set(SRC ImGuiPFR.cpp)

# define the imguipfr target
sfml_add_example(imguipfr GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)

set_property(TARGET imguipfr PROPERTY CXX_STANDARD 23)

find_package(Boost 1.70 REQUIRED COMPONENTS pfr)
target_link_libraries(imguipfr PRIVATE Boost::pfr)
