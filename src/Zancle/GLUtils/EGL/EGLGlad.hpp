#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp" // IWYU pragma: keep

// Emscripten does not use GLAD
#ifdef ZA_SYSTEM_EMSCRIPTEN
    #define KHRONOS_APIENTRY
    #include <EGL/egl.h>
#else
    #include <glad/egl.h>
#endif
