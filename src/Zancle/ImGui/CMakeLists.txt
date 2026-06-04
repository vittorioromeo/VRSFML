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

if(ZA_OS_IOS)
    target_link_libraries(zancle-imgui PRIVATE "-framework OpenGLES")
elseif(ZA_OS_ANDROID OR ZA_OS_EMSCRIPTEN OR ZA_OPENGL_ES)
    find_package(EGL REQUIRED)
    find_package(GLES REQUIRED)
    target_link_libraries(zancle-imgui PRIVATE EGL::EGL GLES::GLES)
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(zancle-imgui PRIVATE OpenGL::GL)
endif()

# use an immediately invoked function to scope option variables we have to set
function(zancle_add_imgui_dependencies)
    # remember whether we are building Zancle as a shared library
    if(BUILD_SHARED_LIBS)
        set(ZA_BUILD_SHARED_LIBS ON)
    endif()

    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(BUILD_SHARED_LIBS OFF)

    CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY vittorioromeo/imgui
        GIT_TAG 1ed7d681aad270ebf53bba7f3b906509a53cd1f9
        SYSTEM TRUE
    )

    # imgui has no CMake support, so we create our own target

    file(GLOB imgui_sources "${imgui_SOURCE_DIR}/*.cpp")

    set(IMGUI_FOUND TRUE)
    set(IMGUI_INCLUDE_DIRS ${imgui_SOURCE_DIR})
    set(IMGUI_SOURCES ${imgui_sources} ${imgui_sources_cpp})
    set(IMGUI_VERSION "1.91.0")

    if(BUILD_SHARED_LIBS)
        add_library(imgui STATIC ${IMGUI_SOURCES})
    else()
        add_library(imgui ${IMGUI_SOURCES})
    endif()

    target_compile_features(imgui PUBLIC cxx_std_23)

    target_sources(imgui PUBLIC
                    FILE_SET public_headers
                    TYPE HEADERS
                    BASE_DIRS ${IMGUI_INCLUDE_DIRS}
                    FILES ${IMGUI_INCLUDE_DIRS}/imgui.h)

    set_target_properties(imgui PROPERTIES FOLDER "Dependencies")

    target_compile_options(imgui PRIVATE "-w")

    target_include_directories(imgui
                                SYSTEM PUBLIC $<BUILD_INTERFACE:${IMGUI_INCLUDE_DIRS}>
                                                $<INSTALL_INTERFACE:include>)

    target_link_libraries(imgui PRIVATE Zancle::Graphics)

    # add flags required for emscripten support (e.g. pthread)
    zancle_apply_emscripten_options(imgui)

    # if building Zancle as a shared library and linking our dependencies in
    # as static libraries we need to build them with -fPIC
    if(ZA_BUILD_SHARED_LIBS)
        set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()

    zancle_set_stdlib(imgui)
    add_library(ImGui::ImGui ALIAS imgui)

    # TODO P0: this is hardcoded, should follow same pattern as Macros.cmake
    install(TARGETS imgui EXPORT ZancleImGuiStaticTargets FILE_SET public_headers)
endfunction()

zancle_add_imgui_dependencies()

target_link_libraries(zancle-imgui PRIVATE ImGui::ImGui)
target_compile_definitions(zancle-imgui PUBLIC IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/include/Zancle/ImGui/ImConfigSFML.hpp")
