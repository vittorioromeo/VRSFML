#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Define the SFML version
////////////////////////////////////////////////////////////
#define SFML_VERSION_MAJOR      3
#define SFML_VERSION_MINOR      0
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

// iOS
#define SFML_SYSTEM_IOS

#elif TARGET_OS_MAC

// macOS
#define SFML_SYSTEM_MACOS

#else

// Unsupported Apple system
#error This Apple operating system is not supported by SFML library

#endif

#elif defined(__unix__)

// UNIX system, see which one it is
#if defined(__ANDROID__)

// Android
#define SFML_SYSTEM_ANDROID

#elif defined(__linux__)

// Linux
#define SFML_SYSTEM_LINUX

#elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)

// FreeBSD
#define SFML_SYSTEM_FREEBSD

#elif defined(__OpenBSD__)

// OpenBSD
#define SFML_SYSTEM_OPENBSD

#elif defined(__NetBSD__)

// NetBSD
#define SFML_SYSTEM_NETBSD

#elif defined(__EMSCRIPTEN__)

// Emscripten
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
// Define a portable debug macro
////////////////////////////////////////////////////////////
#if !defined(NDEBUG)

#define SFML_DEBUG

#endif


////////////////////////////////////////////////////////////
// Define helpers to create portable import / export macros for each module
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
