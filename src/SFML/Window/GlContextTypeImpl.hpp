#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/Window/Glad.hpp"


#if defined(SFML_SYSTEM_WINDOWS)

    #if defined(SFML_OPENGL_ES)

        #include "SFML/Window/EGL/EGLContext.hpp"
using DerivedGlContextType = sf::priv::EglContext;

    #else

        #include "SFML/Window/Win32/WglContext.hpp"
using DerivedGlContextType = sf::priv::WglContext;

    #endif

#elif defined(SFML_SYSTEM_LINUX_OR_BSD)

    #if defined(SFML_USE_DRM)

        #include "SFML/Window/DRM/DRMContext.hpp"
using DerivedGlContextType = sf::priv::DRMContext;

    #elif defined(SFML_OPENGL_ES)

        #include "SFML/Window/EGL/EGLContext.hpp"
using DerivedGlContextType = sf::priv::EglContext;

    #else

        #include "SFML/Window/Unix/GlxContext.hpp"
using DerivedGlContextType = sf::priv::GlxContext;

    #endif

#elif defined(SFML_SYSTEM_MACOS)

    #include "SFML/Window/macOS/SFContext.hpp"
using DerivedGlContextType = sf::priv::SFContext;

#elif defined(SFML_SYSTEM_IOS)

    #include "SFML/Window/iOS/EaglContext.hpp"
using DerivedGlContextType = sf::priv::EaglContext;

#elif defined(SFML_SYSTEM_ANDROID)

    #include "SFML/Window/EGL/EGLContext.hpp"
using DerivedGlContextType = sf::priv::EglContext;

#elif defined(SFML_SYSTEM_EMSCRIPTEN)

    #include "SFML/Window/EGL/EGLContext.hpp"
using DerivedGlContextType = sf::priv::EglContext;

#endif

#if defined(SFML_SYSTEM_WINDOWS)

using glEnableFuncType      = void(GLAPIENTRY*)(GLenum);
using glGetErrorFuncType    = GLenum(GLAPIENTRY*)();
using glGetIntegervFuncType = void(GLAPIENTRY*)(GLenum, GLint*);
using glGetStringFuncType   = const GLubyte*(GLAPIENTRY*)(GLenum);
using glGetStringiFuncType  = const GLubyte*(GLAPIENTRY*)(GLenum, GLuint);
using glIsEnabledFuncType   = GLboolean(GLAPIENTRY*)(GLenum);

#else

using glEnableFuncType      = void (*)(GLenum);
using glGetErrorFuncType    = GLenum (*)();
using glGetIntegervFuncType = void (*)(GLenum, GLint*);
using glGetStringFuncType   = const GLubyte* (*)(GLenum);
using glGetStringiFuncType  = const GLubyte* (*)(GLenum, GLuint);
using glIsEnabledFuncType   = GLboolean (*)(GLenum);

#endif

#if !defined(GL_MULTISAMPLE)
    #define GL_MULTISAMPLE 0x80'9D
#endif

#if !defined(GL_MAJOR_VERSION)
    #define GL_MAJOR_VERSION 0x82'1B
#endif

#if !defined(GL_MINOR_VERSION)
    #define GL_MINOR_VERSION 0x82'1C
#endif

#if !defined(GL_NUM_EXTENSIONS)
    #define GL_NUM_EXTENSIONS 0x82'1D
#endif

#if !defined(GL_CONTEXT_FLAGS)
    #define GL_CONTEXT_FLAGS 0x82'1E
#endif

#if !defined(GL_FRAMEBUFFER_SRGB)
    #define GL_FRAMEBUFFER_SRGB 0x8D'B9
#endif

#if !defined(GL_CONTEXT_FLAG_DEBUG_BIT)
    #define GL_CONTEXT_FLAG_DEBUG_BIT 0x00'00'00'02
#endif

#if !defined(GL_CONTEXT_PROFILE_MASK)
    #define GL_CONTEXT_PROFILE_MASK 0x91'26
#endif

#if !defined(GL_CONTEXT_CORE_PROFILE_BIT)
    #define GL_CONTEXT_CORE_PROFILE_BIT 0x00'00'00'01
#endif

#if !defined(GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
    #define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00'00'00'02
#endif
