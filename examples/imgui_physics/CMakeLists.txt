# all source files
set(SRC ImGuiPhysics.cpp)

# Pull Box2D (v3, C-based) for this example only
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BOX2D_SAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS OFF CACHE BOOL "" FORCE)
set(BOX2D_PROFILE OFF CACHE BOOL "" FORCE)
set(BOX2D_VALIDATE OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_DISABLE_SIMD ON CACHE BOOL "" FORCE)

CPMAddPackage(
    NAME box2d
    GITHUB_REPOSITORY erincatto/box2d
    GIT_TAG 8c65dcb91a5e8fbce53492c169cb4460b53b0b54
    SYSTEM TRUE
)

if(TARGET box2d)
    set_target_properties(box2d PROPERTIES FOLDER "Dependencies")
    target_compile_options(box2d PRIVATE "-w")

    # Emscripten requires every TU to share the pthreads/atomics/bulk-memory feature flags,
    # otherwise wasm-ld refuses to link with `--shared-memory`.
    if(SFML_OS_EMSCRIPTEN)
        target_compile_options(box2d PRIVATE ${SFML_EMSCRIPTEN_TARGET_COMPILE_OPTIONS})
        target_link_options(box2d PRIVATE ${SFML_EMSCRIPTEN_TARGET_LINK_OPTIONS})
    endif()
endif()

# define the imgui_physics target
sfml_add_example(imgui_physics GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics SFML::ImGui ImGui::ImGui box2d)
