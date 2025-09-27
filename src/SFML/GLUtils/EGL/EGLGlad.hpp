#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

// Emscripten does not use GLAD
#ifdef SFML_SYSTEM_EMSCRIPTEN
    #define KHRONOS_APIENTRY
    #include <EGL/egl.h>
#else
    #include <glad/egl.h>
#endif
