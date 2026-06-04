// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp" // IWYU pragma: keep

// Emscripten does not use GLAD
#ifndef ZA_SYSTEM_EMSCRIPTEN

    // We check for this definition in order to avoid multiple definitions of GLAD
    // entities during unity builds of Zancle.
    #ifndef GLAD_EGL_IMPLEMENTATION_INCLUDED
        #define GLAD_EGL_IMPLEMENTATION_INCLUDED
        #define GLAD_EGL_IMPLEMENTATION
        #include <glad/egl.h>
    #endif

#endif
