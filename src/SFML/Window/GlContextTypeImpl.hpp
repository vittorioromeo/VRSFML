////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <glad/gl.h>


#if defined(SFML_SYSTEM_WINDOWS)

#if defined(SFML_OPENGL_ES)

#include <SFML/Window/EglContext.hpp>
using DerivedGlContextType = sf::priv::EglContext;

#else

#include <SFML/Window/Win32/WglContext.hpp>
using DerivedGlContextType = sf::priv::WglContext;

#endif

#elif defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD) || defined(SFML_SYSTEM_OPENBSD) || \
    defined(SFML_SYSTEM_NETBSD)

#if defined(SFML_USE_DRM)

#include <SFML/Window/DRM/DRMContext.hpp>
using DerivedGlContextType = sf::priv::DRMContext;

#elif defined(SFML_OPENGL_ES)

#include <SFML/Window/EglContext.hpp>
using DerivedGlContextType = sf::priv::EglContext;

#else

#include <SFML/Window/Unix/GlxContext.hpp>
using DerivedGlContextType = sf::priv::GlxContext;

#endif

#elif defined(SFML_SYSTEM_MACOS)

#include <SFML/Window/macOS/SFContext.hpp>
using DerivedGlContextType = sf::priv::SFContext;

#elif defined(SFML_SYSTEM_IOS)

#include <SFML/Window/iOS/EaglContext.hpp>
using DerivedGlContextType = sf::priv::EaglContext;

#elif defined(SFML_SYSTEM_ANDROID)

#include <SFML/Window/EglContext.hpp>
using DerivedGlContextType = sf::priv::EglContext;

#endif

#if defined(SFML_SYSTEM_WINDOWS)

using glEnableFuncType      = void(APIENTRY*)(GLenum);
using glGetErrorFuncType    = GLenum(APIENTRY*)();
using glGetIntegervFuncType = void(APIENTRY*)(GLenum, GLint*);
using glGetStringFuncType   = const GLubyte*(APIENTRY*)(GLenum);
using glGetStringiFuncType  = const GLubyte*(APIENTRY*)(GLenum, GLuint);
using glIsEnabledFuncType   = GLboolean(APIENTRY*)(GLenum);

#else

using glEnableFuncType      = void (*)(GLenum);
using glGetErrorFuncType    = GLenum (*)();
using glGetIntegervFuncType = void (*)(GLenum, GLint*);
using glGetStringFuncType   = const GLubyte* (*)(GLenum);
using glGetStringiFuncType  = const GLubyte* (*)(GLenum, GLuint);
using glIsEnabledFuncType   = GLboolean (*)(GLenum);

#endif

#if !defined(GL_MULTISAMPLE)
#define GL_MULTISAMPLE 0x809D
#endif

#if !defined(GL_MAJOR_VERSION)
#define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
#define GL_MINOR_VERSION 0x821C
#endif

#if !defined(GL_NUM_EXTENSIONS)
#define GL_NUM_EXTENSIONS 0x821D
#endif

#if !defined(GL_CONTEXT_FLAGS)
#define GL_CONTEXT_FLAGS 0x821E
#endif

#if !defined(GL_FRAMEBUFFER_SRGB)
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

#if !defined(GL_CONTEXT_FLAG_DEBUG_BIT)
#define GL_CONTEXT_FLAG_DEBUG_BIT 0x00000002
#endif

#if !defined(GL_CONTEXT_PROFILE_MASK)
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif

#if !defined(GL_CONTEXT_CORE_PROFILE_BIT)
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#if !defined(GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif
