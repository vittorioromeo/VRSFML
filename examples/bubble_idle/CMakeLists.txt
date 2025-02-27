# all source files
set(SRC BubbleIdle.cpp Serialization.cpp Steam.cpp)

# define the bubble_idle target
sfml_add_example(bubble_idle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui
                         "${CMAKE_CURRENT_SOURCE_DIR}/steam_api64.lib")

target_include_directories(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/public")
