#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wnull-dereference" // Work around MinGW warning
#endif
// #define MA_DEBUG_OUTPUT // Uncomment to enable miniaudio debug output to console

// clang-format off
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <miniaudio.c>
// clang-format on
