#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoModeImpl.hpp"

#include "SFML/System/Android/Activity.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Vector2.hpp"

#include <mutex>


namespace sf::priv
{
////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    // Get the activity states
    priv::ActivityStates& states = priv::getActivity();

    const std::lock_guard lock(states.mutex);
    const auto            desktop = VideoMode(Vector2u(states.fullScreenSize));

    // Return both portrait and landscape resolutions
    return {desktop, VideoMode(Vector2u(desktop.size.y, desktop.size.x), desktop.bitsPerPixel)};
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    // Get the activity states
    priv::ActivityStates& states = priv::getActivity();
    const std::lock_guard lock(states.mutex);

    return VideoMode(Vector2u(states.screenSize));
}

} // namespace sf::priv
