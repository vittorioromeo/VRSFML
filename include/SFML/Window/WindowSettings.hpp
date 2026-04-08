#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/ContextSettings.hpp"

#include "SFML/System/UnicodeString.hpp"
#include "SFML/System/Vec2Base.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup window
/// \brief Aggregate of all parameters used to create a window
///
/// `WindowSettings` is the single struct that
/// `sf::WindowBase::create` and `sf::Window::create` consume
/// to build a new window. It bundles the dimensions and pixel
/// depth, the title, the high-level look-and-feel flags
/// (fullscreen / resizable / closable / titlebar), the
/// rendering pacing options (vsync / framerate limit), and an
/// embedded `ContextSettings` describing the OpenGL context
/// to attach to the window.
///
/// All members have sensible defaults so that
/// `WindowSettings{.size = {800u, 600u}, .title = "Hello"}`
/// is enough for the common case. The advanced
/// `contextSettings` field can usually be left at its default
/// unless you are doing custom OpenGL rendering.
///
/// \see `sf::Window`, `sf::WindowBase`, `sf::ContextSettings`
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] WindowSettings
{
    Vec2u         size;                     //!< Window inner size in pixels (width, height)
    unsigned int  bitsPerPixel{32u};        //!< Pixel depth, in bits per pixel
    UnicodeString title{"Untitled Window"}; //!< Title displayed in the window's title bar
    bool          fullscreen{false};        //!< Whether the window is created in fullscreen mode
    bool          resizable{true};          //!< Whether the window is resizable by the user
    bool          closable{true};           //!< Whether the window has a system-provided close button
    bool          hasTitlebar{true};        //!< Whether the window has a title bar / borders
    bool          vsync{false};             //!< Whether vertical sync is enabled at creation time
    unsigned int  frametimeLimit{0u};       //!< Framerate limit, in FPS (`0` for no limit)
    ContextSettings contextSettings{}; //!< OpenGL context creation parameters (depth/stencil bits, version, profile, ...)
};

// TODO P0: add refresh rate and other fields

} // namespace sf
