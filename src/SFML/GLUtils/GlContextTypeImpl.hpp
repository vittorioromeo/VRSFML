#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


////////////////////////////////////////////////////////////
#ifdef SFML_SYSTEM_EMSCRIPTEN

    #include "SFML/GLUtils/EGL/EGLContext.hpp"
using DerivedGlContextType = sf::priv::EglContext;

#else

    #include "SFML/Window/SDLGlContext.hpp"
using DerivedGlContextType = sf::priv::SDLGlContext;

#endif
