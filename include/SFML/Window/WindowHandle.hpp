#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Config.hpp"


// Windows' HWND is a type alias for struct HWND__*
#ifdef SFML_SYSTEM_WINDOWS
struct HWND__; // NOLINT(bugprone-reserved-identifier)
#endif

namespace sf
{
#ifdef SFML_SYSTEM_WINDOWS

// Window handle is HWND (HWND__*) on Windows
using WindowHandle = HWND__*;

#elifdef SFML_SYSTEM_LINUX_OR_BSD

// Window handle is Window (unsigned long) on Unix - X11
using WindowHandle = unsigned long;

#elifdef SFML_SYSTEM_MACOS

// Window handle is NSWindow or NSView (void*) on macOS - Cocoa
using WindowHandle = void*;

#elifdef SFML_SYSTEM_IOS

// Window handle is UIWindow (void*) on iOS - UIKit
using WindowHandle = void*;

#elifdef SFML_SYSTEM_ANDROID

// Window handle is ANativeWindow* (void*) on Android
using WindowHandle = void*;

#elifdef SFML_SYSTEM_EMSCRIPTEN

// Window handle is string on Emscripten
using WindowHandle = const char*;

#elifdef SFML_DOXYGEN

// Define type alias symbol so that Doxygen can attach some documentation to it
using WindowHandle = "platform-specific";

#endif

} // namespace sf


////////////////////////////////////////////////////////////
/// \typedef sf::WindowHandle
/// \ingroup window
///
/// Low-level window handle type, specific to each platform.
///
/// Platform        | Type
/// ----------------|------------------------------------------------------------
/// Windows         | \p HWND
/// Linux/FreeBSD   | \p %Window (X11)
/// macOS           | either \p NSWindow* or \p NSView*, disguised as \p void*
/// iOS             | \p UIWindow*, disguised as \p void*
/// Android         | \p ANativeWindow*, disguised as \p void*
/// Emscripten      | CSS selector string (e.g. `"#canvas"`)
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
