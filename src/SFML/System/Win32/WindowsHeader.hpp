#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS 0x06'02 // NOLINT(bugprone-reserved-identifier)
#endif

#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x06'02
#endif

#ifndef WINVER
    #define WINVER 0x06'02
#endif

#ifndef UNICODE
    #define UNICODE 1
#endif

#ifndef _UNICODE
    #define _UNICODE 1 // NOLINT(bugprone-reserved-identifier)
#endif

#include <windows.h>
