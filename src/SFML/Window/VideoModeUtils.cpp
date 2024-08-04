#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/VideoModeImpl.hpp>
#include <SFML/Window/VideoModeUtils.hpp>

#include <SFML/Base/Algorithm.hpp>

#include <algorithm>


namespace sf
{
////////////////////////////////////////////////////////////
VideoMode VideoModeUtils::getDesktopMode()
{
    // Directly forward to the OS-specific implementation
    return priv::VideoModeImpl::getDesktopMode();
}


////////////////////////////////////////////////////////////
const std::vector<VideoMode>& VideoModeUtils::getFullscreenModes()
{
    static const auto modes = []
    {
        std::vector<VideoMode> result = priv::VideoModeImpl::getFullscreenModes();
        std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return modes;
}

} // namespace sf
