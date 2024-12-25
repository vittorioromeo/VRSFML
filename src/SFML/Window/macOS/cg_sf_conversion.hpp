#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoMode.hpp"

#include <ApplicationServices/ApplicationServices.h>

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Get bpp of a video mode for OS 10.6 or later
///
/// With OS 10.6 and later, Quartz doesn't use dictionaries any more
/// to represent video mode. Instead it uses a CGDisplayMode opaque type.
///
////////////////////////////////////////////////////////////
unsigned int modeBitsPerPixel(CGDisplayModeRef mode);

////////////////////////////////////////////////////////////
/// \brief Convert a Quartz video mode into a sf::VideoMode object
///
////////////////////////////////////////////////////////////
VideoMode convertCGModeToSFMode(CGDisplayModeRef cgmode);

} // namespace sf::priv
