set(BASE_INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Base)
set(BASE_SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Base)

file(GLOB_RECURSE SRC_BASE CONFIGURE_DEPENDS "${BASE_INCROOT}/*.hpp"  "${BASE_SRCROOT}/*.hpp" "${BASE_SRCROOT}/*.cpp")

set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/System)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/System)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

list(APPEND SRC ${SRC_BASE})
# list(APPEND SRC ${PROJECT_SOURCE_DIR}/tracy/public/tracy/Tracy.hpp)
# include_directories(${PROJECT_SOURCE_DIR}/tracy/public/tracy/)
# add_definitions(-include Tracy.hpp)
# add_definitions(-DTRACY_ENABLE)

if(SFML_ENABLE_PCH)
    list(APPEND SRC ${PROJECT_SOURCE_DIR}/src/SFML/PCH/PCH.cpp)
endif()

# add platform specific sources
if(SFML_OS_WINDOWS)
    file(GLOB PLATFORM_SRC CONFIGURE_DEPENDS "${SRCROOT}/Win32/*.cpp" "${SRCROOT}/Win32/*.hpp" "${SRCROOT}/Win32/*.inl")
else()
    file(GLOB PLATFORM_SRC CONFIGURE_DEPENDS "${SRCROOT}/Unix/*.cpp" "${SRCROOT}/Unix/*.hpp" "${SRCROOT}/Unix/*.inl")

    if(SFML_OS_ANDROID)
        file(GLOB ANDROID_SRC CONFIGURE_DEPENDS "${SRCROOT}/Android/*.cpp" "${SRCROOT}/Android/*.hpp" "${SRCROOT}/Android/*.inl")
        list(APPEND PLATFORM_SRC ANDROID_SRC)
    endif()
endif()

find_package(Threads REQUIRED)

# define the sfml-system target
sfml_add_library(System
                 SOURCES ${SRC} ${PLATFORM_SRC})

# stb_rect_pack sources
target_include_directories(sfml-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/stb_rect_pack")

# enable precompiled headers
if (SFML_ENABLE_PCH)
    message(VERBOSE "enabling PCH for SFML library 'sfml-system' (reused as the PCH for other SFML libraries)")
    target_precompile_headers(sfml-system PRIVATE ${PROJECT_SOURCE_DIR}/src/SFML/PCH.hpp)
endif()

if(SFML_OS_ANDROID)
    # glad sources
    target_include_directories(sfml-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")
endif()

target_link_libraries(sfml-system PRIVATE Threads::Threads)

if(SFML_OS_LINUX)
    target_link_libraries(sfml-system PRIVATE rt)
elseif(SFML_OS_WINDOWS)
    target_link_libraries(sfml-system PRIVATE winmm)
elseif(SFML_OS_ANDROID)
    target_link_libraries(sfml-system PRIVATE android log)
endif()

if(SFML_ENABLE_STACK_TRACES AND NOT SFML_OS_EMSCRIPTEN)
    target_link_libraries(sfml-system PUBLIC cpptrace::cpptrace)
endif()
