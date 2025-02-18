#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

// Emscripten does not use GLAD
#ifndef SFML_SYSTEM_EMSCRIPTEN

// We check for this definition in order to avoid multiple definitions of GLAD
// entities during unity builds of SFML.
#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_EGL_IMPLEMENTATION
#include <glad/egl.h>
#endif // GLAD_IMPL_UTIL_C_

#endif // SFML_SYSTEM_EMSCRIPTEN
