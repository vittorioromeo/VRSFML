// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/VideoMode.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Sort.hpp"
#include "SFML/Base/Vector.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
float VideoModeUtils::getPrimaryDisplayContentScale()
{
    // TODO P0: per display?
    return priv::getSDLLayerSingleton().getPrimaryDisplayContentScale();
}


////////////////////////////////////////////////////////////
VideoMode VideoModeUtils::getDesktopMode()
{
    auto& sdlLayer = priv::getSDLLayerSingleton();

    const auto* desktopDisplayMode = sdlLayer.getPrimaryDisplayDesktopDisplayMode();
    if (desktopDisplayMode == nullptr)
    {
        priv::err() << "`getDesktopMode` failed, returning default video mode";
        return {};
    }

    return sdlLayer.getVideoModeFromSDLDisplayMode(*desktopDisplayMode);
}


////////////////////////////////////////////////////////////
base::Span<const VideoMode> VideoModeUtils::getFullscreenModes()
{
    static const auto cachedModes = []
    {
        base::Vector<VideoMode> result;

        auto& sdlLayer = priv::getSDLLayerSingleton();

        const auto displays = sdlLayer.getDisplays();
        if (!displays.valid() || displays.size() == 0)
            return result;

        const auto displayId = displays[0];
        const auto modes     = sdlLayer.getFullscreenDisplayModesForDisplay(displayId);

        if (!modes.valid())
            return result;

        for (const auto* mode : modes)
        {
            SFML_BASE_ASSERT(mode != nullptr);

            const sf::VideoMode res = sdlLayer.getVideoModeFromSDLDisplayMode(*mode);

            if (base::find(result.begin(), result.end(), res) == result.end())
                result.pushBack(res);
        }

        base::quickSort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return {cachedModes.data(), cachedModes.size()};
}

} // namespace sf
