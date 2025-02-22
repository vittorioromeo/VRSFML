#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoModeImpl.hpp"

#include "SFML/System/Win32/WindowsHeader.hpp"

#include "SFML/Base/Algorithm.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    // Enumerate all available video modes for the primary display adapter
    DEVMODE win32Mode;
    win32Mode.dmSize        = sizeof(win32Mode);
    win32Mode.dmDriverExtra = 0;

    for (int count = 0; EnumDisplaySettings(nullptr, static_cast<DWORD>(count), &win32Mode); ++count)
    {
        // Convert to sf::VideoMode
        const VideoMode mode({win32Mode.dmPelsWidth, win32Mode.dmPelsHeight}, win32Mode.dmBitsPerPel);

        // Add it only if it is not already in the array
        if (base::find(modes.begin(), modes.end(), mode) == modes.end())
            modes.push_back(mode);
    }

    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    DEVMODE win32Mode;
    win32Mode.dmSize        = sizeof(win32Mode);
    win32Mode.dmDriverExtra = 0;

    EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &win32Mode);

    return VideoMode({win32Mode.dmPelsWidth, win32Mode.dmPelsHeight}, win32Mode.dmBitsPerPel);
}

} // namespace sf::priv
