cmake_minimum_required(VERSION 3.28)
project(test-sfml-install CXX)

# This skips the find_package call when building via add_subdirectory since that will fail under those circumstances
if(PROJECT_IS_TOP_LEVEL)
    if(NOT BUILD_SHARED_LIBS)
        set(SFML_STATIC_LIBRARIES ON)
    endif()

    # cmake by default will only search inside the iOS SDK for packages/libraries, so we need to tell it to look elsewhere
    if(IOS)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
    endif()

    find_package(SFML 3.1.0 EXACT CONFIG REQUIRED COMPONENTS Graphics Network Audio)
endif()

add_executable(test-sfml-install Install.cpp)
target_link_libraries(test-sfml-install PRIVATE SFML::ImGui SFML::Graphics SFML::Network SFML::Audio)

# target_include_directories(test-sfml-install PRIVATE ${PROJECT_SOURCE_DIR}/../../tracy/public/tracy/)
# target_compile_options(test-sfml-install PUBLIC -include Tracy.hpp)
# target_compile_definitions(test-sfml-install PUBLIC -DTRACY_ENABLE)

if (SFML_ENABLE_PCH)
    target_precompile_headers(test-sfml-install REUSE_FROM sfml-system)
endif()

if(SFML_USE_STATIC_STD_LIBS)
    set_target_properties(test-sfml-install PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
