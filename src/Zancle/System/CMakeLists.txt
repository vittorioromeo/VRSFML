set(BASE_INCROOT ${PROJECT_SOURCE_DIR}/include/ZancleBase)
set(BASE_SRCROOT ${PROJECT_SOURCE_DIR}/src/ZancleBase)

file(GLOB_RECURSE SRC_BASE CONFIGURE_DEPENDS "${BASE_INCROOT}/*.hpp"  "${BASE_SRCROOT}/*.hpp" "${BASE_SRCROOT}/*.cpp")

set(INCROOT ${PROJECT_SOURCE_DIR}/include/Zancle/System)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/System)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

list(APPEND SRC ${SRC_BASE})
# list(APPEND SRC ${PROJECT_SOURCE_DIR}/tracy/public/tracy/Tracy.hpp)
# include_directories(${PROJECT_SOURCE_DIR}/tracy/public/tracy/)
# add_definitions(-include Tracy.hpp)
# add_definitions(-DTRACY_ENABLE)

if(ZA_ENABLE_PCH)
    list(APPEND SRC ${PROJECT_SOURCE_DIR}/src/Zancle/PCH/PCH.cpp)
endif()

# add platform specific sources
if(ZA_OS_WINDOWS)
    file(GLOB PLATFORM_SRC CONFIGURE_DEPENDS "${SRCROOT}/Win32/*.cpp" "${SRCROOT}/Win32/*.hpp" "${SRCROOT}/Win32/*.inl")
else()
    file(GLOB PLATFORM_SRC CONFIGURE_DEPENDS "${SRCROOT}/Unix/*.cpp" "${SRCROOT}/Unix/*.hpp" "${SRCROOT}/Unix/*.inl")

    if(ZA_OS_ANDROID)
        file(GLOB ANDROID_SRC CONFIGURE_DEPENDS "${SRCROOT}/Android/*.cpp" "${SRCROOT}/Android/*.hpp" "${SRCROOT}/Android/*.inl")
        list(APPEND PLATFORM_SRC ANDROID_SRC)
    endif()
endif()

find_package(Threads REQUIRED)

# define the zancle-system target
zancle_add_library(System
                 SOURCES ${SRC} ${PLATFORM_SRC}
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
    target_include_directories(zancle-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")
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
