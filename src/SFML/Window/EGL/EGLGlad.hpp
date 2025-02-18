#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

// Emscripten does not use GLAD
#ifdef SFML_SYSTEM_EMSCRIPTEN
#define KHRONOS_APIENTRY
#include <EGL/egl.h>
#else
#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_EGL_IMPLEMENTATION
#include <glad/egl.h>
#endif // GLAD_IMPL_UTIL_C_
#endif // SFML_SYSTEM_EMSCRIPTEN
