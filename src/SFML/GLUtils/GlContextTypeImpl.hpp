#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#include "SFML/GLUtils/Glad.hpp"

////////////////////////////////////////////////////////////
#ifdef SFML_SYSTEM_EMSCRIPTEN

    #include "SFML/GLUtils/EGL/EGLContext.hpp"
using DerivedGlContextType = sf::priv::EglContext;

#else

    #include "SFML/Window/SDLGlContext.hpp"
using DerivedGlContextType = sf::priv::SDLGlContext;

#endif


////////////////////////////////////////////////////////////
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
