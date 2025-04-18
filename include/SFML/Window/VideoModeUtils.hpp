#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
/// \brief VideoMode defines a video mode (width, height, bpp)
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API VideoModeUtils
{
public:
    // TODO P0:
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
/// TODO P1: docs
///
/// \see sf::VideoMode
///
////////////////////////////////////////////////////////////
