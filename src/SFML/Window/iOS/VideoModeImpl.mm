#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoModeImpl.hpp"
#include "SFML/Window/iOS/SFAppDelegate.hpp"

#include <UIKit/UIKit.h>

namespace sf::priv
{
////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    const VideoMode desktop = getDesktopMode();

    // Return both portrait and landscape resolutions
    return {desktop, VideoMode(Vector2u(desktop.size.y, desktop.size.x), desktop.bitsPerPixel)};
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    const CGRect bounds       = [[UIScreen mainScreen] bounds];
    const double backingScale = [SFAppDelegate getInstance].backingScaleFactor;
    return VideoMode({static_cast<unsigned int>(bounds.size.width * backingScale),
                      static_cast<unsigned int>(bounds.size.height * backingScale)});
}

} // namespace sf::priv
