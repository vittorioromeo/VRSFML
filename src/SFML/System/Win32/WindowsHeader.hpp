#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINDOWS
// clang-format off
    #define _WIN32_WINDOWS 0x0602 // NOLINT(bugprone-reserved-identifier)
// clang-format on
#endif

#ifndef _WIN32_WINNT
// clang-format off
    #define _WIN32_WINNT 0x0602
// clang-format on
#endif

#ifndef WINVER
// clang-format off
    #define WINVER 0x0602
// clang-format on
#endif

#ifndef UNICODE
    #define UNICODE 1
#endif

#ifndef _UNICODE
    #define _UNICODE 1 // NOLINT(bugprone-reserved-identifier)
#endif

#include <windows.h>
