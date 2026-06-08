# UTILITY_MODULES is declared at the top-level CMakeLists.txt so the install
# helper (and the framework branch) can enumerate the modules from one source.

set(SRC "")
foreach(mod ${UTILITY_MODULES})
    set(MOD_INCROOT ${PROJECT_SOURCE_DIR}/include/Zancle/${mod})
    set(MOD_SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/${mod})

    file(GLOB_RECURSE MOD_SRC CONFIGURE_DEPENDS
        "${MOD_INCROOT}/*.hpp"
        "${MOD_INCROOT}/*.inl"
        "${MOD_SRCROOT}/*.hpp"
        "${MOD_SRCROOT}/*.inl"
        "${MOD_SRCROOT}/*.cpp"
    )

    # Files under any `Unity/` subdirectory are unity-build bundles included
    # by sibling `*Unity.cpp` host translation units, not compiled standalone.
    list(FILTER MOD_SRC EXCLUDE REGEX "/Unity/")
    list(FILTER MOD_SRC EXCLUDE REGEX "/Android/")

    list(APPEND SRC ${MOD_SRC})
endforeach()

if(ZA_ENABLE_PCH)
    list(APPEND SRC ${PROJECT_SOURCE_DIR}/src/Zancle/PCH/PCH.cpp)
endif()

# define the zancle-system target (aggregates all utility modules)
zancle_add_library(System
                 SOURCES ${SRC}
                 DEPENDENCIES "Dependencies.cmake.in")

# moodycamel concurrent queue sources
target_include_directories(zancle-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/moodycamel")

# stb_rect_pack sources
target_include_directories(zancle-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/stb_rect_pack")

# enable precompiled headers
if (ZA_ENABLE_PCH)
    message(VERBOSE "enabling PCH for Zancle library 'zancle-system' (reused as the PCH for other Zancle libraries)")
    target_precompile_headers(zancle-system PRIVATE ${PROJECT_SOURCE_DIR}/src/Zancle/PCH.hpp)
endif()

if(ZA_OS_ANDROID)
    # glad sources
    target_link_libraries(zancle-system PRIVATE $<BUILD_INTERFACE:zancle-glad-headers>)
endif()

target_link_libraries(zancle-system PRIVATE Threads::Threads)

if(ZA_OS_LINUX)
    target_link_libraries(zancle-system PRIVATE rt)
elseif(ZA_OS_WINDOWS)
    target_link_libraries(zancle-system PRIVATE winmm synchronization)
elseif(ZA_OS_ANDROID)
    target_link_libraries(zancle-system PRIVATE android log)
endif()

if(ZA_ENABLE_STACK_TRACES AND NOT ZA_OS_EMSCRIPTEN)
    target_link_libraries(zancle-system PRIVATE $<BUILD_INTERFACE:zancle::libbacktrace>)
endif()
