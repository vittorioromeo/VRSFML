#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief VideoMode defines a video mode (width, height, bpp)
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API VideoMode
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructors initializes all members to 0.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] VideoMode() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct the video mode with its attributes
    ///
    /// \param modeSize         Width and height in pixels
    /// \param modeBitsPerPixel Pixel depths in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit VideoMode(Vector2u modeSize, unsigned int modeBitsPerPixel = 32);

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
    /// The returned array is sorted from best to worst, so that
    /// the first element will always give the best mode (higher
    /// width, height and bits-per-pixel).
    ///
    /// \return Array containing all the supported fullscreen modes
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static const std::vector<VideoMode>& getFullscreenModes();

    ////////////////////////////////////////////////////////////
    /// \brief Tell whether or not the video mode is valid
    ///
    /// The validity of video modes is only relevant when using
    /// fullscreen windows; otherwise any video mode can be used
    /// with no restriction.
    ///
    /// \return True if the video mode is valid for fullscreen mode
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isValid() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vector2u     size;           //!< Video mode width and height, in pixels
    unsigned int bitsPerPixel{}; //!< Video mode pixel depth, in bits per pixels
};

////////////////////////////////////////////////////////////
/// \relates VideoMode
/// \brief Overload of == operator to compare two video modes
///
/// \param left  Left operand (a video mode)
/// \param right Right operand (a video mode)
///
/// \return True if modes are equal
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool operator==(const VideoMode& left, const VideoMode& right);

////////////////////////////////////////////////////////////
/// \relates VideoMode
/// \brief Overload of != operator to compare two video modes
///
/// \param left  Left operand (a video mode)
/// \param right Right operand (a video mode)
///
/// \return True if modes are different
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool operator!=(const VideoMode& left, const VideoMode& right);

////////////////////////////////////////////////////////////
/// \relates VideoMode
/// \brief Overload of < operator to compare video modes
///
/// \param left  Left operand (a video mode)
/// \param right Right operand (a video mode)
///
/// \return True if \a left is lesser than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool operator<(const VideoMode& left, const VideoMode& right);

////////////////////////////////////////////////////////////
/// \relates VideoMode
/// \brief Overload of > operator to compare video modes
///
/// \param left  Left operand (a video mode)
/// \param right Right operand (a video mode)
///
/// \return True if \a left is greater than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool operator>(const VideoMode& left, const VideoMode& right);

////////////////////////////////////////////////////////////
/// \relates VideoMode
/// \brief Overload of <= operator to compare video modes
///
/// \param left  Left operand (a video mode)
/// \param right Right operand (a video mode)
///
/// \return True if \a left is lesser or equal than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool operator<=(const VideoMode& left, const VideoMode& right);

////////////////////////////////////////////////////////////
/// \relates VideoMode
/// \brief Overload of >= operator to compare video modes
///
/// \param left  Left operand (a video mode)
/// \param right Right operand (a video mode)
///
/// \return True if \a left is greater or equal than \a right
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool operator>=(const VideoMode& left, const VideoMode& right);

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::VideoMode
/// \ingroup window
///
/// A video mode is defined by a width and a height (in pixels)
/// and a depth (in bits per pixel). Video modes are used to
/// setup windows (sf::Window) at creation time.
///
/// The main usage of video modes is for fullscreen mode:
/// indeed you must use one of the valid video modes
/// allowed by the OS (which are defined by what the monitor
/// and the graphics card support), otherwise your window
/// creation will just fail.
///
/// sf::VideoMode provides a static function for retrieving
/// the list of all the video modes supported by the system:
/// getFullscreenModes().
///
/// A custom video mode can also be checked directly for
/// fullscreen compatibility with its isValid() function.
///
/// Additionally, sf::VideoMode provides a static function
/// to get the mode currently used by the desktop: getDesktopMode().
/// This allows to build windows with the same size or pixel
/// depth as the current resolution.
///
/// Usage example:
/// \code
/// // Display the list of all the video modes available for fullscreen
/// std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
/// for (std::size_t i = 0; i < modes.size(); ++i)
/// {
///     sf::VideoMode mode = modes[i];
///     std::cout << "Mode #" << i << ": "
///               << mode.size.x << "x" << mode.size.y << " - "
///               << mode.bitsPerPixel << " bpp\n";
/// }
///
/// // Create a window with the same pixel depth as the desktop
/// sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
/// window.create(sf::VideoMode({1024, 768}, desktop.bitsPerPixel), "SFML window");
/// \endcode
///
////////////////////////////////////////////////////////////
