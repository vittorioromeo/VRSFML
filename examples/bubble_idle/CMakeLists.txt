# all source files
if (NOT SFML_OS_EMSCRIPTEN)
    set(SRC BubbleIdle.cpp Serialization.cpp Steam.cpp)

    # set the correct steam library
    if (SFML_OS_WINDOWS)
        set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/steam_api64.lib")
    else()
        set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libsteam_api.so")
    endif()
else()
    set(SRC BubbleIdle.cpp Serialization.cpp)
    set(STEAM_LIB "")
endif()

# define the bubble_idle target
sfml_add_example(bubble_idle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui ${STEAM_LIB})

if (WIN32)
    target_link_libraries(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.res")
endif()

target_include_directories(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/public")

if (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(bubble_idle PRIVATE "-Wl,-subsystem,windows" "-mwindows")
    target_link_libraries(bubble_idle PRIVATE "-Wl,-subsystem,windows" "-mwindows")
endif()
