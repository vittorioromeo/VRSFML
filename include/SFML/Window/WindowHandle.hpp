#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"

// Windows' HWND is a type alias for struct HWND__*
#if defined(SFML_SYSTEM_WINDOWS)
struct HWND__; // NOLINT(bugprone-reserved-identifier)
#endif

namespace sf
{
#if defined(SFML_SYSTEM_WINDOWS)

// Window handle is HWND (HWND__*) on Windows
using WindowHandle = HWND__*;

#elif defined(SFML_SYSTEM_LINUX_OR_BSD)

// Window handle is Window (unsigned long) on Unix - X11
using WindowHandle = unsigned long;

#elif defined(SFML_SYSTEM_MACOS)

// Window handle is NSWindow or NSView (void*) on macOS - Cocoa
using WindowHandle = void*;

#elif defined(SFML_SYSTEM_IOS)

// Window handle is UIWindow (void*) on iOS - UIKit
using WindowHandle = void*;

#elif defined(SFML_SYSTEM_ANDROID)

// Window handle is ANativeWindow* (void*) on Android
using WindowHandle = void*;

#elif defined(SFML_SYSTEM_EMSCRIPTEN)

// Window handle is int on Emscripten
using WindowHandle = int;

#elif defined(SFML_DOXYGEN)

// Define type alias symbol so that Doxygen can attach some documentation to it
using WindowHandle = "platform-specific";

#endif

} // namespace sf


////////////////////////////////////////////////////////////
/// \typedef sf::WindowHandle
/// \ingroup window
///
/// Define a low-level window handle type, specific to
/// each platform.
///
/// Platform        | Type
/// ----------------|------------------------------------------------------------
/// Windows         | \p HWND
/// Linux/FreeBSD   | \p %Window
/// macOS           | either \p NSWindow* or \p NSView*, disguised as \p void*
/// iOS             | \p UIWindow*
/// Android         | \p ANativeWindow*
///
/// \par macOS Specification
///
/// On macOS, a `sf::Window` can be created either from an
/// existing \p NSWindow* or an \p NSView*. When the window
/// is created from a window, SFML will use its content view
/// as the OpenGL area. `sf::Window::getNativeHandle()` will
/// return the handle that was used to create the window,
/// which is a \p NSWindow* by default.
///
////////////////////////////////////////////////////////////
