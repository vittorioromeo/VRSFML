set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Window)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Window)

file(GLOB SRC CONFIGURE_DEPENDS "${INCROOT}/*.hpp" "${INCROOT}/*.inl"  "${SRCROOT}/*.hpp" "${SRCROOT}/*.inl" "${SRCROOT}/*.cpp" "${SRCROOT}/Stub/*.hpp" "${SRCROOT}/Stub/*.cpp")

find_package(Threads REQUIRED)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)

set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
set(SDL_EXAMPLES OFF)

set(SDL_CCACHE ON)

set(SDL_AUDIO OFF)
set(SDL_CAMERA OFF)
set(SDL_DIALOG OFF)
set(SDL_FILESYSTEM OFF)
set(SDL_FSOPS OFF)
set(SDL_GPU OFF)
set(SDL_HIDAPI OFF)
set(SDL_LOCALE OFF)
set(SDL_MISC OFF)
set(SDL_POWER OFF)
set(SDL_PROCESS OFF)
set(SDL_RENDER OFF)
set(SDL_SENSOR OFF)
set(SDL_STORAGE OFF)
set(SDL_TRAY OFF)

CPMAddPackage(
    NAME sdl
    GITHUB_REPOSITORY vittorioromeo/SDL
    GIT_TAG b43cca1a2ecbddf83dc0888d4f8c3a7c733368c1
    SYSTEM TRUE
)

get_property(sdl3_include_dirs TARGET SDL_uclibc PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
set_property(TARGET SDL_uclibc PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${sdl3_include_dirs}")
target_compile_options(SDL_uclibc PRIVATE "-w" "-pthread")
set_target_properties(SDL_uclibc PROPERTIES UNITY_BUILD OFF)
target_link_libraries(SDL_uclibc PRIVATE Threads::Threads)

if (TARGET SDL3-static)
    get_property(sdl3_include_dirs TARGET SDL3-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    set_property(TARGET SDL3-static PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${sdl3_include_dirs}")
    target_compile_options(SDL3-static PRIVATE "-w" "-pthread")
    set_target_properties(SDL3-static PROPERTIES UNITY_BUILD OFF)
    target_link_libraries(SDL3-static PRIVATE Threads::Threads)
endif()

if (TARGET SDL3-dynamic)
    get_property(sdl3_include_dirs TARGET SDL3-dynamic PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    set_property(TARGET SDL3-dynamic PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${sdl3_include_dirs}")
    target_compile_options(SDL3-dynamic PRIVATE "-w" "-pthread")
    set_target_properties(SDL3-dynamic PROPERTIES UNITY_BUILD OFF)
    target_link_libraries(SDL3-dynamic PRIVATE Threads::Threads)
endif()



# add platform specific sources
if(SFML_OS_WINDOWS)
    set(PLATFORM_SRC
        ${SRCROOT}/Win32/Utils.hpp
        ${SRCROOT}/Win32/Win32ImplUnity.cpp

        # ${SRCROOT}/Win32/Unity/Utils.cpp
        # ${SRCROOT}/Win32/Unity/VulkanImplWin32.cpp
    )
elseif(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    set(PLATFORM_SRC
        ${SRCROOT}/Unix/Display.cpp
        ${SRCROOT}/Unix/Display.hpp
        ${SRCROOT}/Unix/Utils.hpp
        ${SRCROOT}/Unix/VulkanImplX11.cpp
    )
elseif(SFML_OS_MACOS)
    enable_language(OBJC OBJCXX)
    set(PLATFORM_SRC
        ${SRCROOT}/macOS/cg_sf_conversion.hpp
        ${SRCROOT}/macOS/cg_sf_conversion.mm
        ${SRCROOT}/macOS/HIDInputManager.hpp
        ${SRCROOT}/macOS/HIDInputManager.mm
        ${SRCROOT}/macOS/NSImage+raw.h
        ${SRCROOT}/macOS/NSImage+raw.mm
        ${SRCROOT}/macOS/Scaling.h
        ${SRCROOT}/macOS/SensorImpl.hpp
        ${SRCROOT}/macOS/SFApplication.h
        ${SRCROOT}/macOS/SFApplication.m
        ${SRCROOT}/macOS/SFApplicationDelegate.h
        ${SRCROOT}/macOS/SFApplicationDelegate.m
        ${SRCROOT}/macOS/SFKeyboardModifiersHelper.h
        ${SRCROOT}/macOS/SFKeyboardModifiersHelper.mm
        ${SRCROOT}/macOS/SFOpenGLView.h
        ${SRCROOT}/macOS/SFOpenGLView.mm
        ${SRCROOT}/macOS/SFOpenGLView+keyboard.mm
        ${SRCROOT}/macOS/SFOpenGLView+keyboard_priv.h
        ${SRCROOT}/macOS/SFOpenGLView+mouse.mm
        ${SRCROOT}/macOS/SFOpenGLView+mouse_priv.h
        ${SRCROOT}/macOS/SFSilentResponder.h
        ${SRCROOT}/macOS/SFSilentResponder.m
        ${SRCROOT}/macOS/SFWindow.h
        ${SRCROOT}/macOS/SFWindow.m
        ${SRCROOT}/macOS/SFWindowController.h
        ${SRCROOT}/macOS/SFWindowController.mm
        ${SRCROOT}/macOS/SFViewController.h
        ${SRCROOT}/macOS/SFViewController.mm
        ${SRCROOT}/macOS/WindowImplDelegateProtocol.h
        ${SRCROOT}/macOS/AutoreleasePoolWrapper.hpp
        ${SRCROOT}/macOS/AutoreleasePoolWrapper.mm
    )
elseif(SFML_OS_IOS)
    enable_language(OBJCXX)
    set(PLATFORM_SRC
        ${SRCROOT}/iOS/SensorImpl.mm
        ${SRCROOT}/iOS/SensorImpl.hpp
        ${SRCROOT}/iOS/ObjCType.hpp
        ${SRCROOT}/iOS/SFAppDelegate.hpp
        ${SRCROOT}/iOS/SFAppDelegate.mm
        ${SRCROOT}/iOS/SFView.hpp
        ${SRCROOT}/iOS/SFView.mm
        ${SRCROOT}/iOS/SFViewController.hpp
        ${SRCROOT}/iOS/SFViewController.mm
        ${SRCROOT}/iOS/SFMain.hpp
        ${SRCROOT}/iOS/SFMain.mm
    )
elseif(SFML_OS_ANDROID)
    set(PLATFORM_SRC
        ${SRCROOT}/Android/SensorImpl.cpp
        ${SRCROOT}/Android/SensorImpl.hpp
    )
endif()

# define the sfml-window target
sfml_add_library(Window
                 SOURCES ${SRC} ${PLATFORM_SRC}
                 DEPENDENCIES "Dependencies.cmake.in")
# DRM libraries
if(SFML_OS_LINUX OR SFML_OS_FREEBSD OR SFML_OS_OPENBSD OR SFML_OS_NETBSD)
    if(SFML_USE_DRM)
        find_package(DRM REQUIRED)
        find_package(GBM REQUIRED)
        target_link_libraries(sfml-window PRIVATE DRM::DRM GBM::GBM)
    else()
        find_package(X11 REQUIRED COMPONENTS Xrandr Xcursor Xi)
        target_link_libraries(sfml-window PRIVATE X11::X11 X11::Xrandr X11::Xcursor X11::Xi)
    endif()
endif()


# SDL3
target_link_libraries(sfml-window PUBLIC SFML::System PRIVATE $<BUILD_INTERFACE:SDL3::SDL3> SFML::GLUtils)


# glad sources
target_include_directories(sfml-window SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# When static linking on macOS, we need to add this flag for objective C to work
# https://developer.apple.com/library/archive/qa/qa1490/_index.html
if((NOT BUILD_SHARED_LIBS) AND SFML_OS_MACOS)
    target_link_libraries(sfml-window PRIVATE -ObjC)
endif()

# Vulkan headers
target_include_directories(sfml-window SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/vulkan")

if(SFML_OS_IOS)
    target_link_libraries(sfml-window PRIVATE "-framework OpenGLES")
elseif(SFML_OS_ANDROID OR SFML_OS_EMSCRIPTEN OR SFML_OPENGL_ES)
    find_package(EGL REQUIRED)
    find_package(GLES REQUIRED)
    target_link_libraries(sfml-window PRIVATE EGL::EGL GLES::GLES)
else()
    find_package(OpenGL REQUIRED COMPONENTS OpenGL)
    target_link_libraries(sfml-window PRIVATE OpenGL::GL)
endif()

if(SFML_OS_WINDOWS AND NOT SFML_COMPILER_MSVC)
    include(CheckIncludeFileCXX)
    check_include_file_cxx(dinput.h DINPUT_H_FOUND)
    if(NOT DINPUT_H_FOUND)
        target_include_directories(sfml-window SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/mingw")
    endif()
endif()

if(SFML_OS_LINUX)
    find_package(UDev REQUIRED)
    target_link_libraries(sfml-window PRIVATE UDev::UDev dl)
elseif(SFML_OS_WINDOWS)
    target_link_libraries(sfml-window PRIVATE winmm gdi32)
elseif(SFML_OS_FREEBSD)
    target_link_libraries(sfml-window PRIVATE usbhid)
elseif(SFML_OS_MACOS)
    target_link_libraries(sfml-window PRIVATE "-framework Foundation" "-framework AppKit" "-framework IOKit" "-framework Carbon")
elseif(SFML_OS_IOS)
    target_link_libraries(sfml-window PUBLIC "-framework Foundation" "-framework UIKit" "-framework CoreGraphics" "-framework QuartzCore" "-framework CoreMotion")
elseif(SFML_OS_ANDROID)
    target_link_libraries(sfml-window PRIVATE android)
endif()

# on some platforms (e.g. Raspberry Pi 3 armhf), GCC requires linking libatomic to use <atomic> features
# that aren't supported by native CPU instructions (64-bit atomic operations on 32-bit architecture)
if(SFML_COMPILER_GCC)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles("#include <atomic>
        int main(){std::atomic_ullong x(1); return x.fetch_add(1);}" ATOMIC_TEST)
    if(NOT ATOMIC_TEST)
        target_link_libraries(sfml-window PRIVATE atomic)
    endif()
endif()
