# ===========================================================================
# ExampleUtils library
#
# General-purpose example helpers (Sampler, Scaling, Fmt, LoadedSound, ...).
# Profiling lives in its own pair of libraries below.
# ===========================================================================
set(INCROOT ${PROJECT_SOURCE_DIR}/examples/include/ExampleUtils)
set(SRCROOT ${PROJECT_SOURCE_DIR}/examples/src/ExampleUtils)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl" "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

if(NOT SFML_BUILD_AUDIO)
    list(REMOVE_ITEM SRC "${SRCROOT}/LoadedSound.cpp")
endif()

add_library(ExampleUtils STATIC ${SRC})
target_link_libraries(ExampleUtils PUBLIC SFML::System SFML::Window SFML::Graphics)
target_include_directories(ExampleUtils PRIVATE ${PROJECT_SOURCE_DIR}/examples/include)

if(SFML_BUILD_AUDIO)
    target_link_libraries(ExampleUtils PRIVATE SFML::Audio)
endif()

# add flags required for emscripten support (e.g. pthread)
sfml_apply_emscripten_options(ExampleUtils)


# ===========================================================================
# ExampleProfiler libraries (two flavors: enabled / disabled)
#
# Both compile the same source(s) but with different `SFEX_PROFILER_ENABLED`
# states. Each example chooses one (or links neither, if it doesn't include
# the profiler header at all):
#
#     target_link_libraries(my_example PRIVATE ExampleProfilerEnabled)
#     target_link_libraries(my_example PRIVATE ExampleProfilerDisabled)
#
# The `SFEX_PROFILER_ENABLED` macro is `PUBLIC` on the enabled variant, so
# linking it propagates the define to every consumer TU (no manual
# `#define SFEX_PROFILER_ENABLED` needed). The link-check tag in
# `Profiler.hpp` then guarantees the consumer's macro state matches the
# library variant it linked against -- mismatches surface as
# `undefined reference to sfex::priv::sfexProfilerLinkCheck*()`.
# ===========================================================================
if(SFML_BUILD_IMGUI)
    set(PROFILER_INCROOT ${PROJECT_SOURCE_DIR}/examples/include/ExampleProfiler)
    set(PROFILER_SRCROOT ${PROJECT_SOURCE_DIR}/examples/src/ExampleProfiler)

    file(GLOB PROFILER_SRC CONFIGURE_DEPENDS
        "${PROFILER_INCROOT}/*.hpp"
        "${PROFILER_INCROOT}/*.inl"
        "${PROFILER_SRCROOT}/*.hpp"
        "${PROFILER_SRCROOT}/*.inl"
        "${PROFILER_SRCROOT}/*.cpp")

    function(sfml_add_example_profiler target_name enable_macro)
        add_library(${target_name} STATIC ${PROFILER_SRC})

        target_link_libraries(${target_name} PUBLIC SFML::System SFML::Window SFML::Graphics)
        target_link_libraries(${target_name} PRIVATE SFML::ImGui ImGui::ImGui)
        target_link_libraries(${target_name} PUBLIC ExampleUtils) # for `ExampleUtils/Sampler.hpp`

        target_include_directories(${target_name} PRIVATE ${PROJECT_SOURCE_DIR}/examples/include)

        if(${enable_macro})
            # PUBLIC so every consumer TU of this variant gets the define.
            target_compile_definitions(${target_name} PUBLIC SFEX_PROFILER_ENABLED)
        endif()

        # add flags required for emscripten support (e.g. pthread)
        sfml_apply_emscripten_options(${target_name})
    endfunction()

    sfml_add_example_profiler(ExampleProfilerEnabled  TRUE)
    sfml_add_example_profiler(ExampleProfilerDisabled FALSE)
endif()

# CLI based examples
if(NOT SFML_OS_IOS AND NOT SFML_OS_EMSCRIPTEN)
    if(SFML_BUILD_NETWORK)
        add_subdirectory(ftp)
        add_subdirectory(http)
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
        add_subdirectory(coroutine)
        add_subdirectory(coroutine_dialogue)
        add_subdirectory(coroutine_tutorial)
        add_subdirectory(imgui_minimal)
        add_subdirectory(imgui_physics)
        add_subdirectory(island)
        add_subdirectory(joystick)
        add_subdirectory(particles)
        add_subdirectory(pixel_perfect)
        add_subdirectory(rockets)
        add_subdirectory(shader)
        add_subdirectory(showcase)
        add_subdirectory(tetris_survivors)

        if (NOT SFML_OS_EMSCRIPTEN)
            add_subdirectory(imgui_multiple_windows)
            add_subdirectory(vulkan)
        endif()
    endif()

    if(SFML_OS_WINDOWS)
        add_subdirectory(win32)
        add_subdirectory(raw_input)
    elseif(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD)
        if(NOT SFML_USE_DRM)
            add_subdirectory(raw_input)
        endif()
    endif()
endif()

if(SFML_BUILD_GRAPHICS AND SFML_BUILD_AUDIO)
    add_subdirectory(tennis)
    add_subdirectory(sound_effects)
    add_subdirectory(keyboard)
endif()
