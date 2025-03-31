#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/SDLLayer.hpp"
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/Base/Algorithm.hpp"

#include <algorithm> // std::sort
#include <vector>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::VideoMode sdlModeToVideoMode(const SDL_DisplayMode* mode)
{
    if (mode == nullptr)
        return {};

    const auto* info = sf::priv::getSDLLayerSingleton().getPixelFormatDetails(mode->format);

    if (info == nullptr)
    {
        sf::priv::err() << "Failed to get pixel format details for display mode";
        return {};
    }

    return {
        .size         = sf::Vector2i(mode->w, mode->h).toVector2u(),
        .bitsPerPixel = static_cast<unsigned int>(info->bits_per_pixel),
    };
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
float VideoMode::getDPIAwareScalingFactor()
{
    // TODO P0: per display?
    return priv::getSDLLayerSingleton().getDPIAwareScalingFactor();
}


////////////////////////////////////////////////////////////
VideoMode VideoModeUtils::getDesktopMode()
{
    auto& sdlLayer = priv::getSDLLayerSingleton();
    return sdlModeToVideoMode(sdlLayer.getDesktopDisplayMode(sdlLayer.getDisplays()[0]));
}


////////////////////////////////////////////////////////////
base::Span<const VideoMode> VideoModeUtils::getFullscreenModes()
{
    static const auto cachedModes = []
    {
        std::vector<VideoMode> result;

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
            const sf::VideoMode res = sdlModeToVideoMode(mode);

            if (base::find(result.begin(), result.end(), res) == result.end())
                result.push_back(res);
        }

        std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return {cachedModes.data(), cachedModes.size()};
}

} // namespace sf
