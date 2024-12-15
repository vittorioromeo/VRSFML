#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoModeImpl.hpp"
#include "SFML/Window/macOS/cg_sf_conversion.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Algorithm.hpp"

namespace sf::priv
{

////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    // Retrieve all modes available for main screen only.
    CFArrayRef cgmodes = CGDisplayCopyAllDisplayModes(CGMainDisplayID(), nullptr);

    if (cgmodes == nullptr)
    {
        sf::priv::err() << "Couldn't get VideoMode for main display.";
        return {};
    }

    const VideoMode        desktop = getDesktopMode();
    std::vector<VideoMode> modes   = {desktop};

    // Loop on each mode and convert it into a sf::VideoMode object.
    const CFIndex modesCount = CFArrayGetCount(cgmodes);
    for (CFIndex i = 0; i < modesCount; ++i)
    {
        auto* cgmode = static_cast<CGDisplayModeRef>(const_cast<void*>(CFArrayGetValueAtIndex(cgmodes, i)));

        const VideoMode mode = convertCGModeToSFMode(cgmode);

        // Skip if bigger than desktop as we currently don't perform hard resolution switch
        if ((mode.size.x > desktop.size.x) || (mode.size.y > desktop.size.y))
            continue;

        // If not yet listed we add it to our modes array.
        if (base::find(modes.begin(), modes.end(), mode) == modes.end())
            modes.push_back(mode);
    }

    // Clean up memory.
    CFRelease(cgmodes);

    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    VideoMode mode; // RVO

    // Rely exclusively on mode and convertCGModeToSFMode
    // instead of display id and CGDisplayPixelsHigh/Wide.

    const CGDirectDisplayID display = CGMainDisplayID();
    CGDisplayModeRef        cgmode  = CGDisplayCopyDisplayMode(display);

    mode = convertCGModeToSFMode(cgmode);

    CGDisplayModeRelease(cgmode);

    return mode;
}

} // namespace sf::priv
