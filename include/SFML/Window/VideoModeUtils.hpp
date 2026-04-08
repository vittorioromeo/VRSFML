#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/Base/Span.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
struct VideoMode;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Static helpers that query the system about video modes
///
/// Where `sf::VideoMode` is the plain data describing one
/// video mode (size, bit depth, refresh rate, pixel density),
/// `VideoModeUtils` is the small set of static functions that
/// talk to the OS to enumerate the modes the current display
/// supports and to retrieve display-level information such as
/// the desktop mode and content scale.
///
/// \see `sf::VideoMode`
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API VideoModeUtils
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Get the content scale of the primary display
    ///
    /// On HiDPI displays the OS reports a content scale factor
    /// (e.g. `2.f` for a "@2x" Retina display) that applications
    /// should use to size their UI. This function returns that
    /// factor for the primary display, or `1.f` on systems
    /// without HiDPI support.
    ///
    /// \return Content scale factor of the primary display
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static float getPrimaryDisplayContentScale();

    ////////////////////////////////////////////////////////////
    /// \brief Get the current desktop video mode
    ///
    /// \return Current desktop video mode
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static VideoMode getDesktopMode();

    ////////////////////////////////////////////////////////////
    /// \brief Retrieve all the video modes supported in fullscreen mode
    ///
    /// When creating a fullscreen window, the video mode is restricted
    /// to be compatible with what the graphics driver and monitor
    /// support. This function returns the complete list of all video
    /// modes that can be used in fullscreen mode.
    /// The returned span is sorted from best to worst, so that
    /// the first element will always give the best mode (higher
    /// width, height and bits-per-pixel).
    ///
    /// \return Span containing all the supported fullscreen modes
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Span<const VideoMode> getFullscreenModes();
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::VideoModeUtils
/// \ingroup window
///
/// `sf::VideoModeUtils` exposes a few static helpers built on
/// top of the underlying SDL/OS layer to enumerate available
/// video modes, query the desktop's current mode, and obtain
/// the HiDPI content scale factor.
///
/// Together with `sf::VideoMode`, it covers everything you
/// need to set up a fullscreen `sf::Window`:
/// \code
/// const auto modes = sf::VideoModeUtils::getFullscreenModes();
/// // modes[0] is the best mode supported by the system
///
/// const auto desktop = sf::VideoModeUtils::getDesktopMode();
/// // desktop is the mode currently in use on the primary display
/// \endcode
///
/// \see `sf::VideoMode`
///
////////////////////////////////////////////////////////////
