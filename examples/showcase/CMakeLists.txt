set(SRC
    Showcase.cpp
    ShowcaseShapes.cpp
    ShowcaseBunnyMark.cpp
    ShowcaseAudio.cpp
    ShowcaseIndividualShape.cpp
    ShowcaseTextComparison.cpp)

sfml_add_example(showcase GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui)
