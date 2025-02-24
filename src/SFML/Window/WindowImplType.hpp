#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


////////////////////////////////////////////////////////////
#define SFML_PRIV_DEFINE_WINDOWIMPLTYPE(implType) \
    namespace sf::priv                            \
    {                                             \
                                                  \
    class WindowImplType : public implType        \
    {                                             \
    public:                                       \
        using implType::implType;                 \
    };                                            \
                                                  \
    } // namespace sf::priv


////////////////////////////////////////////////////////////
#if defined(SFML_SYSTEM_WINDOWS)

    #include "SFML/Window/Win32/WindowImplWin32.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplWin32)
    #include "SFML/Window/VulkanImpl.hpp"

#elif defined(SFML_SYSTEM_LINUX_OR_BSD)

    #if defined(SFML_USE_DRM)

        #include "SFML/Window/DRM/WindowImplDRM.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplDRM)
        #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

    #else

        #include "SFML/Window/Unix/WindowImplX11.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplX11)
        #include "SFML/Window/VulkanImpl.hpp"

    #endif

#elif defined(SFML_SYSTEM_MACOS)

    #include "SFML/Window/macOS/WindowImplCocoa.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplCocoa)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#elif defined(SFML_SYSTEM_IOS)

    #include "SFML/Window/iOS/WindowImplUIKit.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplUIKit)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#elif defined(SFML_SYSTEM_ANDROID)

    #include "SFML/Window/Android/WindowImplAndroid.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplAndroid)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#elif defined(SFML_SYSTEM_EMSCRIPTEN)

    #include "SFML/Window/Emscripten/WindowImplEmscripten.hpp"
SFML_PRIV_DEFINE_WINDOWIMPLTYPE(WindowImplEmscripten)
    #define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#endif


////////////////////////////////////////////////////////////
#undef SFML_PRIV_DEFINE_WINDOWIMPLTYPE
