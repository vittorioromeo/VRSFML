#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////c////////////////////////////
#include "SFML/Window/ContextSettings.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/Vec2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup window
/// \brief Window creation settings
///
/// This struct defines window creation settings such as the
/// size and pixel depth. An optional style can be passed to
/// customize the look and behavior of the window (borders,
/// title bar, resizable, closable, etc...).
///
/// The last field is an optional structure specifying
/// advanced OpenGL context settings such as antialiasing,
/// depth-buffer bits, etc. You shouldn't care about these
/// parameters for a regular usage of the graphics module.
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] WindowSettings
{
    Vec2u           size;                     //!< Video mode width and height, in pixels
    unsigned int    bitsPerPixel{32u};        //!< Video mode pixel depth, in bits per pixels
    String          title{"Untitled Window"}; //!< Title of the window
    bool            fullscreen{false};        //!< Is the window fullscreen?
    bool            resizable{true};          //!< Is the window resizable?
    bool            closable{true};           //!< Is the window closable?
    bool            hasTitlebar{true};        //!< Does the window have a titlebar?
    bool            vsync{false};             //!< Is vertical sync enabled?
    unsigned int    frametimeLimit{0u};       //!< Frametime limit (FPS) (`0` for no limit)
    ContextSettings contextSettings{};        //!< Additional settings for the underlying OpenGL context
};

// TODO P0: add refresh rate and other fields

} // namespace sf
