set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/GLUtils)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/GLUtils)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp")

# add platform specific sources
if(SFML_OS_WINDOWS)
    if(SFML_OPENGL_ES)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/EGL/EGLCheck.cpp
            ${SRCROOT}/EGL/EGLCheck.hpp
            ${SRCROOT}/EGL/EGLContext.cpp
            ${SRCROOT}/EGL/EGLContext.hpp
            ${SRCROOT}/EGL/EGLGlad.cpp
            ${SRCROOT}/EGL/EGLGlad.hpp
        )
    else()
        list(APPEND PLATFORM_SRC
            # ${SRCROOT}/Win32/WglContext.cpp
            # ${SRCROOT}/Win32/WglContext.hpp
        )
    endif()
elseif(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    if(SFML_USE_DRM)
        add_definitions(-DSFML_USE_DRM)
        set(PLATFORM_SRC
            ${SRCROOT}/EGL/EGLCheck.cpp
            ${SRCROOT}/EGL/EGLCheck.hpp
            # ${SRCROOT}/DRM/DRMContext.cpp
            # ${SRCROOT}/DRM/DRMContext.hpp
        )
    else()
        if(SFML_OPENGL_ES)
            list(APPEND PLATFORM_SRC
                ${SRCROOT}/EGL/EGLCheck.cpp
                ${SRCROOT}/EGL/EGLCheck.hpp
                ${SRCROOT}/EGL/EGLContext.cpp
                ${SRCROOT}/EGL/EGLContext.hpp
                ${SRCROOT}/EGL/EGLGlad.cpp
                ${SRCROOT}/EGL/EGLGlad.hpp
            )
        else()
             list(APPEND PLATFORM_SRC
                # ${SRCROOT}/Unix/GlxContext.cpp
                # ${SRCROOT}/Unix/GlxContext.hpp
            )
        endif()
    endif()
elseif(SFML_OS_MACOS)
    enable_language(OBJC OBJCXX)
    set(PLATFORM_SRC
        # ${SRCROOT}/macOS/SFContext.hpp
        # ${SRCROOT}/macOS/SFContext.mm
        # ${SRCROOT}/macOS/SFOpenGLView.h
        # ${SRCROOT}/macOS/SFOpenGLView.mm
        # ${SRCROOT}/macOS/SFOpenGLView+keyboard.mm
        # ${SRCROOT}/macOS/SFOpenGLView+keyboard_priv.h
        # ${SRCROOT}/macOS/SFOpenGLView+mouse.mm
        # ${SRCROOT}/macOS/SFOpenGLView+mouse_priv.h
    )
elseif(SFML_OS_IOS)
    enable_language(OBJCXX)
    set(PLATFORM_SRC
        # ${SRCROOT}/iOS/EaglContext.mm
        # ${SRCROOT}/iOS/EaglContext.hpp
    )
elseif(SFML_OS_ANDROID)
    set(PLATFORM_SRC
        ${SRCROOT}/EGL/EGLCheck.cpp
        ${SRCROOT}/EGL/EGLCheck.hpp
        ${SRCROOT}/EGL/EGLContext.cpp
        ${SRCROOT}/EGL/EGLContext.hpp
    )
elseif(SFML_OS_EMSCRIPTEN)
    set(PLATFORM_SRC
        ${SRCROOT}/EGL/EGLCheck.cpp
        ${SRCROOT}/EGL/EGLCheck.hpp
        ${SRCROOT}/EGL/EGLContext.cpp
        ${SRCROOT}/EGL/EGLContext.hpp
    )
endif()

# define the sfml-glutils target
sfml_add_library(GLUtils
                 SOURCES ${SRC} ${PLATFORM_SRC}
                 DEPENDENCIES "Dependencies.cmake.in")
# DRM libraries
if(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    if(SFML_USE_DRM)
        find_package(DRM REQUIRED)
        find_package(GBM REQUIRED)
        target_link_libraries(sfml-glutils PRIVATE DRM::DRM GBM::GBM)
    else()
        find_package(X11 REQUIRED COMPONENTS Xi)
        target_link_libraries(sfml-glutils PRIVATE X11::X11 X11::Xi)
    endif()
endif()


# glad sources
target_include_directories(sfml-glutils SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# When static linking on macOS, we need to add this flag for objective C to work
# https://developer.apple.com/library/archive/qa/qa1490/_index.html
if((NOT BUILD_SHARED_LIBS) AND SFML_OS_MACOS)
    target_link_libraries(sfml-glutils PRIVATE -ObjC)
endif()

# Vulkan headers
target_include_directories(sfml-glutils SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/vulkan")

if(SFML_OS_IOS)
    target_link_libraries(sfml-glutils PRIVATE "-framework OpenGLES")
elseif(SFML_OS_ANDROID OR SFML_OS_EMSCRIPTEN OR SFML_OPENGL_ES)
    find_package(EGL REQUIRED)
    find_package(GLES REQUIRED)
    target_link_libraries(sfml-glutils PRIVATE EGL::EGL GLES::GLES)
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(sfml-glutils PRIVATE OpenGL::GL)
endif()

if(SFML_OS_WINDOWS)
    target_link_libraries(sfml-glutils PRIVATE winmm gdi32)
elseif(SFML_OS_MACOS)
    target_link_libraries(sfml-glutils PRIVATE "-framework Foundation" "-framework AppKit" "-framework IOKit" "-framework Carbon")
elseif(SFML_OS_IOS)
    target_link_libraries(sfml-glutils PUBLIC "-framework Foundation" "-framework UIKit" "-framework CoreGraphics" "-framework QuartzCore" "-framework CoreMotion")
elseif(SFML_OS_ANDROID)
    target_link_libraries(sfml-glutils PRIVATE android)
endif()
