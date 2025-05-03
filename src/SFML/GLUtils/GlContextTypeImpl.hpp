#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

#ifdef SFML_SYSTEM_EMSCRIPTEN
    #include "SFML/GLUtils/EGL/EGLContext.hpp"
#else
    #include "SFML/Window/SDLGlContext.hpp"
#endif


namespace sf::priv
{
////////////////////////////////////////////////////////////
#ifdef SFML_SYSTEM_EMSCRIPTEN
using DerivedGlContextType = EglContext;
#else
using DerivedGlContextType = SDLGlContext;
#endif

} // namespace sf::priv
