# CLI based examples
if(NOT SFML_OS_IOS AND NOT SFML_OS_EMSCRIPTEN)
    if(SFML_BUILD_NETWORK)
        add_subdirectory(ftp)
        add_subdirectory(sockets)
    endif()
    if(SFML_BUILD_NETWORK AND SFML_BUILD_AUDIO)
        add_subdirectory(voip)
    endif()
    if(SFML_BUILD_AUDIO)
        add_subdirectory(sound)
        add_subdirectory(sound_capture)
        add_subdirectory(sound_multi_device)
    endif()
endif()

# GUI based examples
if(SFML_BUILD_GRAPHICS)
    add_subdirectory(event_handling)
    add_subdirectory(stencil)

    if(NOT SFML_OS_IOS)
        add_subdirectory(arkanoid)
        add_subdirectory(batching_benchmark)
        add_subdirectory(batching)
        add_subdirectory(block_puzzle)
        add_subdirectory(bubble_idle)
        add_subdirectory(imgui_minimal)
        add_subdirectory(island)
        add_subdirectory(joystick)
        add_subdirectory(particles)
        add_subdirectory(shader)
        add_subdirectory(showcase)
        add_subdirectory(text_benchmark)

        if (NOT SFML_OS_EMSCRIPTEN)
            add_subdirectory(imgui_multiple_windows)
            add_subdirectory(vulkan)
        endif()
    endif()

    if(SFML_OS_WINDOWS)
        add_subdirectory(win32)
        add_subdirectory(raw_input)
    elseif(SFML_OS_LINUX OR SFML_OS_FREEBSD)
        if(NOT SFML_USE_DRM)
            add_subdirectory(raw_input)
            add_subdirectory(x11_example)
        endif()
    elseif(SFML_OS_MACOS)
        add_subdirectory(cocoa)
    endif()
endif()

if(SFML_BUILD_GRAPHICS AND SFML_BUILD_AUDIO)
    add_subdirectory(tennis)
    add_subdirectory(sound_effects)
    add_subdirectory(keyboard)
endif()
