set(INCROOT ${PROJECT_SOURCE_DIR}/include/Zancle/ImGui)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/ImGui)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# define the zancle-imgui target
zancle_add_library(ImGui
                 SOURCES ${SRC})

# glad sources
target_include_directories(zancle-imgui SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# setup dependencies
target_link_libraries(zancle-imgui PUBLIC Zancle::Graphics)

# GL backend comes from cmake/PlatformLibs.cmake.
target_link_libraries(zancle-imgui PRIVATE ${ZA_GL_BACKEND_LIBS})

# imgui has no CMake support, so we create our own target after pulling the
# sources via CPM. Option/policy overrides are scoped via block(); the target
# itself is created outside so subsequent target_* commands operate on it.
block()
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_SHARED_LIBS OFF)

    CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY vittorioromeo/imgui
        GIT_TAG 1ed7d681aad270ebf53bba7f3b906509a53cd1f9
        SYSTEM TRUE
    )

    # block() does not export normal variables, so propagate the paths we need.
    set(IMGUI_INCLUDE_DIRS "${imgui_SOURCE_DIR}" PARENT_SCOPE)
    file(GLOB imgui_sources "${imgui_SOURCE_DIR}/*.cpp")
    set(IMGUI_SOURCES "${imgui_sources}" PARENT_SCOPE)
endblock()

if(BUILD_SHARED_LIBS)
    add_library(imgui STATIC ${IMGUI_SOURCES})
else()
    add_library(imgui ${IMGUI_SOURCES})
endif()

target_sources(imgui PUBLIC
                FILE_SET public_headers
                TYPE HEADERS
                BASE_DIRS ${IMGUI_INCLUDE_DIRS}
                FILES ${IMGUI_INCLUDE_DIRS}/imgui.h)

target_include_directories(imgui
                            SYSTEM PUBLIC $<BUILD_INTERFACE:${IMGUI_INCLUDE_DIRS}>
                                          $<INSTALL_INTERFACE:include>)

target_link_libraries(imgui
    PUBLIC $<BUILD_INTERFACE:zancle-build-options>
    PRIVATE Zancle::Graphics)

zancle_tame_thirdparty(TARGETS imgui)

add_library(ImGui::ImGui ALIAS imgui)

# TODO P0: this is hardcoded, should follow same pattern as Macros.cmake
install(TARGETS imgui EXPORT ZancleImGuiStaticTargets FILE_SET public_headers)

target_link_libraries(zancle-imgui PRIVATE ImGui::ImGui)
target_compile_definitions(zancle-imgui PUBLIC IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/include/Zancle/ImGui/ImConfigSFML.hpp")
