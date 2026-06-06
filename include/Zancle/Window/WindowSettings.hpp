#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/ContextSettings.hpp"

#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/RequireDesignatedInitializers.hpp"


namespace za
{
////////////////////////////////////////////////////////////
/// \ingroup window
/// \brief Aggregate of all parameters used to create a window
///
/// `WindowSettings` is the single struct that
/// `za::WindowBase::create` and `za::Window::create` consume
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
/// \see `za::Window`, `za::WindowBase`, `za::ContextSettings`
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] WindowSettings
{
    ZA_REQUIRE_DESIGNATED_INITIALIZERS;

    za::Optional<Vec2i> position{za::nullOpt};    //!< Initial window position (`za::nullOpt` lets the OS pick)
    Vec2u               size;                     //!< Window inner size in pixels (width, height)
    za::Optional<Vec2u> minimumSize{za::nullOpt}; //!< Initial minimum size enforced on user resizes
    za::Optional<Vec2u> maximumSize{za::nullOpt}; //!< Initial maximum size enforced on user resizes
    unsigned int        bitsPerPixel{32u};        //!< Pixel depth, in bits per pixel
    Utf8String          title{"Untitled Window"}; //!< Title displayed in the window's title bar
    bool                fullscreen{false};        //!< Whether the window is created in fullscreen mode
    bool                resizable{true};          //!< Whether the window is resizable by the user
    bool                closable{true};           //!< Whether the window has a system-provided close button
    bool                hasTitlebar{true};        //!< Whether the window has a title bar / borders
    bool                visible{true};            //!< Whether the window is initially shown
    bool                vsync{false};             //!< Whether vertical sync is enabled at creation time
    unsigned int        frametimeLimit{0u};       //!< Framerate limit, in FPS (`0` for no limit)
    bool                mouseCursorVisible{true}; //!< Whether the mouse cursor is initially visible
    bool                keyRepeatEnabled{true};   //!< Whether key repeat events are initially enabled
    float               joystickThreshold{0.1f};  //!< Initial joystick "move" event threshold, in `[0, 100]`
    ContextSettings     contextSettings{};        //!< OpenGL context creation parameters
};

// TODO P0: add refresh rate and other fields

} // namespace za
