set(SRC
    Showcase.cpp
    ShowcaseShapes.cpp
    ShowcaseBunnyMark.cpp
    ShowcaseAudio.cpp
    ShowcaseIndividualShape.cpp
    ShowcaseTextComparison.cpp)

zancle_add_example(showcase GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio Zancle::Graphics Zancle::ImGui ImGui::ImGui)
