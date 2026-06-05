file(GLOB SRC CONFIGURE_DEPENDS "*.cpp" "*.c")

if(ZA_OS_EMSCRIPTEN OR ZA_ARCH_ARM64)
    list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/Steam.cpp")
    set(STEAM_LIB "")
elseif(ZA_OS_WINDOWS)
    set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/steam_api64.lib")
else()
    set(STEAM_LIB "${CMAKE_CURRENT_SOURCE_DIR}/libsteam_api.so")
endif()

# define the bubble_idle target
zancle_add_example(bubble_idle GUI_APP
                 SOURCES ${SRC}
                 DEPENDS Zancle::Audio Zancle::Graphics Zancle::ImGui ImGui::ImGui ${STEAM_LIB})

if (STEAM_LIB)
    target_compile_definitions(bubble_idle PRIVATE BUBBLEBYTE_USE_STEAMWORKS=1)
endif()

if (WIN32 AND NOT ZA_ARCH_ARM64)
    target_link_libraries(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.res")
endif()

target_include_directories(bubble_idle PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/public")

target_link_libraries(bubble_idle PRIVATE ExampleProfilerEnabled)

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
    zancle_add_example(bubble_idle_demo GUI_APP
                    SOURCES ${SRC}
                    DEPENDS Zancle::Audio Zancle::Graphics Zancle::ImGui ImGui::ImGui ${STEAM_LIB})

    target_compile_definitions(bubble_idle_demo PRIVATE BUBBLEBYTE_DEMO)

    if (WIN32 AND NOT ZA_ARCH_ARM64)
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
