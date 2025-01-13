#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// SFML version
////////////////////////////////////////////////////////////
#define SFML_VERSION_MAJOR      3
#define SFML_VERSION_MINOR      1
#define SFML_VERSION_PATCH      0
#define SFML_VERSION_IS_RELEASE false


////////////////////////////////////////////////////////////
// Identify the operating system
// see https://sourceforge.net/p/predef/wiki/Home/
////////////////////////////////////////////////////////////
#if defined(_WIN32)

// Windows
#define SFML_SYSTEM_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif

#elif defined(__APPLE__) && defined(__MACH__)

// Apple platform, see which one it is
#include "TargetConditionals.h"

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define SFML_SYSTEM_IOS

#elif TARGET_OS_MAC
#define SFML_SYSTEM_MACOS

#else

// Unsupported Apple system
#error This Apple operating system is not supported by SFML library

#endif

#elif defined(__unix__)

// UNIX system, see which one it is
#if defined(__ANDROID__)
#define SFML_SYSTEM_ANDROID

#elif defined(__linux__)
#define SFML_SYSTEM_LINUX
#define SFML_SYSTEM_LINUX_OR_BSD

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define SFML_SYSTEM_FREEBSD
#define SFML_SYSTEM_LINUX_OR_BSD

#elif defined(__OpenBSD__)
#define SFML_SYSTEM_OPENBSD
#define SFML_SYSTEM_LINUX_OR_BSD

#elif defined(__NetBSD__)
#define SFML_SYSTEM_NETBSD
#define SFML_SYSTEM_LINUX_OR_BSD

#elif defined(__EMSCRIPTEN__)
#define SFML_SYSTEM_EMSCRIPTEN

#else

// Unsupported UNIX system
#error This UNIX operating system is not supported by SFML library

#endif

#else

// Unsupported system
#error This operating system is not supported by SFML library

#endif


////////////////////////////////////////////////////////////
// Ensure minimum C++ language standard version is met
////////////////////////////////////////////////////////////
#if (defined(_MSVC_LANG) && _MSVC_LANG < 202002L) || (!defined(_MSVC_LANG) && __cplusplus < 202002L)
#error "Enable C++20 or newer for your compiler (e.g. -std=c++20 for GCC/Clang or /std:c++20 for MSVC)"
#endif


////////////////////////////////////////////////////////////
// Portable debug macro
////////////////////////////////////////////////////////////
#if !defined(NDEBUG)

#define SFML_DEBUG

#endif


////////////////////////////////////////////////////////////
// Helpers to create portable import / export macros for each module
////////////////////////////////////////////////////////////
#if !defined(SFML_STATIC)

#if defined(SFML_SYSTEM_WINDOWS)

// Windows compilers need specific (and different) keywords for export and import
#define SFML_API_EXPORT __declspec(dllexport)
#define SFML_API_IMPORT __declspec(dllimport)

// For Visual C++ compilers, we also need to turn off this annoying C4251 warning
#ifdef _MSC_VER

#pragma warning(disable : 4251)

#endif

#else // Linux, FreeBSD, macOS

#define SFML_API_EXPORT __attribute__((__visibility__("default")))
#define SFML_API_IMPORT __attribute__((__visibility__("default")))

#endif

#else

// Static build doesn't need import/export macros
#define SFML_API_EXPORT
#define SFML_API_IMPORT

#endif
