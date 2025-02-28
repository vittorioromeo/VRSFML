# all source files
set(SRC BubbleIdle.cpp Serialization.cpp Steam.cpp)

# set the correct steam library
if (SFML_OS_WINDOWS)
    set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/steam_api64.lib")
else()
    set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libsteam_api.so")
endif()

# define the bubble_idle target
sfml_add_example(bubble_idle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui ${STEAM_LIB})

target_include_directories(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/public")
