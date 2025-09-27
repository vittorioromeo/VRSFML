// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

// Emscripten does not use GLAD
#ifndef SFML_SYSTEM_EMSCRIPTEN

    // We check for this definition in order to avoid multiple definitions of GLAD
    // entities during unity builds of SFML.
    #ifndef GLAD_EGL_IMPLEMENTATION_INCLUDED
        #define GLAD_EGL_IMPLEMENTATION_INCLUDED
        #define GLAD_EGL_IMPLEMENTATION
        #include <glad/egl.h>
    #endif

#endif
