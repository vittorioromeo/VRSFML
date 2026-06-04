set(INCROOT ${PROJECT_SOURCE_DIR}/include/Zancle/GLUtils)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/GLUtils)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# add platform specific sources
if(ZA_OS_WINDOWS)
    if(ZA_OPENGL_ES)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/EGL/EGLCheck.cpp
            ${SRCROOT}/EGL/EGLCheck.hpp
            ${SRCROOT}/EGL/EGLGlad.cpp
            ${SRCROOT}/EGL/EGLGlad.hpp
        )
    endif()
elseif(ZA_OS_LINUX OR ZA_OS_FREEBSD OR ZA_OS_OPENBSD OR ZA_OS_NETBSD)
    if(ZA_USE_DRM)
        add_definitions(-DZA_USE_DRM)
        set(PLATFORM_SRC
            ${SRCROOT}/EGL/EGLCheck.cpp
            ${SRCROOT}/EGL/EGLCheck.hpp
        )
    else()
        if(ZA_OPENGL_ES)
            list(APPEND PLATFORM_SRC
                ${SRCROOT}/EGL/EGLCheck.cpp
                ${SRCROOT}/EGL/EGLCheck.hpp
                ${SRCROOT}/EGL/EGLGlad.cpp
                ${SRCROOT}/EGL/EGLGlad.hpp
            )
        endif()
    endif()
elseif(ZA_OS_IOS)
    enable_language(OBJCXX)
elseif(ZA_OS_ANDROID)
    set(PLATFORM_SRC
        ${SRCROOT}/EGL/EGLCheck.cpp
        ${SRCROOT}/EGL/EGLCheck.hpp
        ${SRCROOT}/EGL/EGLContext.cpp
        ${SRCROOT}/EGL/EGLContext.hpp
    )
endif()

# define the zancle-glutils target
zancle_add_library(GLUtils
                 SOURCES ${SRC} ${PLATFORM_SRC}
                 DEPENDENCIES "Dependencies.cmake.in")

# DRM libraries
if(ZA_OS_LINUX OR ZA_OS_FREEBSD OR ZA_OS_OPENBSD OR ZA_OS_NETBSD)
    if(ZA_USE_DRM)
        find_package(DRM REQUIRED)
        find_package(GBM REQUIRED)
        target_link_libraries(zancle-glutils PRIVATE DRM::DRM GBM::GBM)
    endif()
endif()


# glad sources
target_include_directories(zancle-glutils SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# When static linking on macOS, we need to add this flag for objective C to work
# https://developer.apple.com/library/archive/qa/qa1490/_index.html
if((NOT BUILD_SHARED_LIBS) AND ZA_OS_MACOS)
    target_link_libraries(zancle-glutils PRIVATE -ObjC)
endif()

# Vulkan headers
target_include_directories(zancle-glutils SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/vulkan")

if(ZA_OS_IOS)
    target_link_libraries(zancle-glutils PRIVATE "-framework OpenGLES")
elseif(ZA_OS_ANDROID OR ZA_OS_EMSCRIPTEN OR ZA_OPENGL_ES)
    find_package(EGL REQUIRED)
    find_package(GLES REQUIRED)
    target_link_libraries(zancle-glutils PRIVATE EGL::EGL GLES::GLES)
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(zancle-glutils PRIVATE OpenGL::GL)
endif()

if(ZA_OS_WINDOWS)
    target_link_libraries(zancle-glutils PRIVATE winmm gdi32)
elseif(ZA_OS_MACOS)
    target_link_libraries(zancle-glutils PRIVATE "-framework Foundation" "-framework AppKit" "-framework IOKit" "-framework Carbon")
elseif(ZA_OS_IOS)
    target_link_libraries(zancle-glutils PUBLIC "-framework Foundation" "-framework UIKit" "-framework CoreGraphics" "-framework QuartzCore" "-framework CoreMotion")
elseif(ZA_OS_ANDROID)
    target_link_libraries(zancle-glutils PRIVATE android)
endif()

if(ZA_FATAL_OPENGL_ERRORS)
    target_compile_definitions(zancle-glutils PRIVATE "ZA_FATAL_OPENGL_ERRORS")
endif()
