# all source files
if (NOT SFML_OS_EMSCRIPTEN AND NOT ARCH_ARM64)
    set(SRC BubbleIdle.cpp
            Serialization.cpp
            Steam.cpp
            CatNames.cpp
            MainApp.cpp
            MainGameplay.cpp
            GameRender.cpp
            GameUpdate.cpp
            MainLoop.cpp
            PlayerInput.cpp
            MainProgression.cpp
            MainRendering.cpp
            MainSystems.cpp
            UIQuickbar.cpp
            UIRuntime.cpp
            UITabBar.cpp
            UIShopCats.cpp
            UIShopCore.cpp
            UIShopGoals.cpp
            UIPrestigeCore.cpp
            UIPrestigeOverview.cpp
            UIPrestigeShrines.cpp
            UISettingsAudio.cpp
            UISettingsData.cpp
            UISettingsDebug.cpp
            UISettingsDisplay.cpp
            UISettingsGraphics.cpp
            UISettingsUi.cpp
            UiCore.cpp
            UiMagic.cpp
            UiPrestige.cpp
            UiSettings.cpp
            UiShop.cpp
            UiStats.cpp)

    # set the correct steam library
    if (SFML_OS_WINDOWS)
        set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/steam_api64.lib")
    else()
        set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libsteam_api.so")
    endif()

    add_definitions(-DBUBBLEBYTE_USE_STEAMWORKS=1)
else()
    set(SRC BubbleIdle.cpp
            Serialization.cpp
            CatNames.cpp
            MainApp.cpp
            MainGameplay.cpp
            GameRender.cpp
            GameUpdate.cpp
            MainLoop.cpp
            PlayerInput.cpp
            MainProgression.cpp
            MainRendering.cpp
            MainSystems.cpp
            UIQuickbar.cpp
            UIRuntime.cpp
            UITabBar.cpp
            UIShopCats.cpp
            UIShopCore.cpp
            UIShopGoals.cpp
            UIPrestigeCore.cpp
            UIPrestigeOverview.cpp
            UIPrestigeShrines.cpp
            UISettingsAudio.cpp
            UISettingsData.cpp
            UISettingsDebug.cpp
            UISettingsDisplay.cpp
            UISettingsGraphics.cpp
            UISettingsUi.cpp
            UiCore.cpp
            UiMagic.cpp
            UiPrestige.cpp
            UiSettings.cpp
            UiShop.cpp
            UiStats.cpp)
    set(STEAM_LIB "")
endif()

# define the bubble_idle target
sfml_add_example(bubble_idle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui ${STEAM_LIB})

if (WIN32 AND NOT ARCH_ARM64)
    target_link_libraries(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.res")
endif()

target_include_directories(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/public")

# set_target_properties(bubble_idle PROPERTIES
#                       UNITY_BUILD ON
#                       UNITY_BUILD_BATCH_SIZE 0)

if (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_options(bubble_idle PRIVATE "-Wl,-subsystem,windows" "-mwindows")
    target_link_libraries(bubble_idle PRIVATE "-Wl,-subsystem,windows" "-mwindows")
endif()

# do not produce demo target by default
if (FALSE)
    # define the bubble_idle_demo target
    sfml_add_example(bubble_idle_demo GUI_APP
                    SOURCES ${SRC}
                    DEPENDS SFML::Audio SFML::Graphics SFML::ImGui ImGui::ImGui ${STEAM_LIB})

    target_compile_definitions(bubble_idle_demo PRIVATE BUBBLEBYTE_DEMO)

    if (WIN32 AND NOT ARCH_ARM64)
        target_link_libraries(bubble_idle_demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.res")
    endif()

    target_include_directories(bubble_idle_demo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/public")

    set_target_properties(bubble_idle_demo PROPERTIES
                          UNITY_BUILD ON
                          UNITY_BUILD_BATCH_SIZE 0)

    if (WIN32 AND ${CMAKE_BUILD_TYPE} STREQUAL "Release")
        target_compile_options(bubble_idle_demo PRIVATE "-Wl,-subsystem,windows" "-mwindows")
        target_link_libraries(bubble_idle_demo PRIVATE "-Wl,-subsystem,windows" "-mwindows")
    endif()
endif()
