# include the Zancle specific macros
include(${PROJECT_SOURCE_DIR}/cmake/Macros.cmake)

# set the output directory for Zancle libraries
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# add the modules subdirectories

# zancle-system
add_subdirectory(System)

# zancle-main
if(ZA_OS_WINDOWS OR ZA_OS_ANDROID OR ZA_OS_IOS)
    add_subdirectory(Main)
endif()

# zancle-glutils
if(ZA_BUILD_GLUTILS OR ZA_BUILD_WINDOW OR ZA_BUILD_GRAPHICS)
    add_subdirectory(GLUtils)
endif()

# zancle-window
if(ZA_BUILD_WINDOW OR ZA_BUILD_GRAPHICS)
    add_subdirectory(Window)
endif()

# zancle-network
if(ZA_BUILD_NETWORK)
    add_subdirectory(Network)
endif()

# zancle-graphics
if(ZA_BUILD_GRAPHICS OR ZA_BUILD_IMGUI)
    add_subdirectory(Graphics)
endif()

# zancle-imgui
if(ZA_BUILD_IMGUI)
    add_subdirectory(ImGui)
endif()

# zancle-audio
if(ZA_BUILD_AUDIO)
    add_subdirectory(Audio)
endif()

# on some platforms (e.g. Raspberry Pi 3 armhf), GCC requires linking libatomic to use <atomic> features
# that aren't supported by native CPU instructions (64-bit atomic operations on 32-bit architecture)
if(ZA_COMPILER_GCC)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("#include <atomic>
        int main(){std::atomic_ullong x(1); return x.fetch_add(1);}" ATOMIC_TEST)
    if(NOT ATOMIC_TEST)
        if(ZA_BUILD_GRAPHICS)
            target_link_libraries(zancle-graphics PRIVATE atomic)
        endif()
        if(ZA_BUILD_IMGUI)
            target_link_libraries(zancle-imgui PRIVATE atomic)
        endif()
        if(ZA_BUILD_AUDIO)
            target_link_libraries(zancle-audio PRIVATE atomic)
        endif()
    endif()
endif()
