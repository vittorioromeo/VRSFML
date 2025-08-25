# all source files
set(SRC Flocking.cpp)

# define the flocking target
sfml_add_example(flocking GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)

                 # define the flocking target
sfml_add_example(flockingOOP GUI_APP
                 SOURCES FlockingOOP.cpp
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)
                                  # define the flocking target
sfml_add_example(flockingAOS GUI_APP
                 SOURCES FlockingAOS.cpp
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui)
