#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/System/Vector2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief VideoMode defines a video mode (width, height, bpp)
///
////////////////////////////////////////////////////////////
struct SFML_WINDOW_API [[nodiscard]] VideoMode
{
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
    Vector2u     size;              //!< Video mode width and height, in pixels
    unsigned int bitsPerPixel{32u}; //!< Video mode pixel depth, in bits per pixels
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
/// std::vector<sf::VideoMode> modes = sf::VideoModeUtils::getFullscreenModes();
/// for (std::size_t i = 0; i < modes.size(); ++i)
/// {
///     sf::VideoMode mode = modes[i];
///     std::cout << "Mode #" << i << ": "
///               << mode.size.x << "x" << mode.size.y << " - "
///               << mode.bitsPerPixel << " bpp\n";
/// }
///
/// // Create a window with the same pixel depth as the desktop
/// sf::VideoMode desktop = sf::VideoModeUtils::getDesktopMode();
/// window.create(sf::VideoMode({1024, 768}, desktop.bitsPerPixel), "SFML window");
/// \endcode
///
////////////////////////////////////////////////////////////
