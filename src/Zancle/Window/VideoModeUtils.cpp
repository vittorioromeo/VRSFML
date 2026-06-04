// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/VideoModeUtils.hpp"

#include "Zancle/Window/SDLLayer.hpp"
#include "Zancle/Window/VideoMode.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/System/Err.hpp"

#include "ZancleBase/Algorithm/Find.hpp"
#include "ZancleBase/Algorithm/Sort.hpp"
#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Span.hpp"
#include "ZancleBase/Vector.hpp"


namespace za
{
////////////////////////////////////////////////////////////
float VideoModeUtils::getPrimaryDisplayContentScale()
{
    // TODO P0: per display?
    return WindowContext::getSDLLayer().getPrimaryDisplayContentScale();
}


////////////////////////////////////////////////////////////
VideoMode VideoModeUtils::getDesktopMode()
{
    auto& sdlLayer = WindowContext::getSDLLayer();

    const auto* desktopDisplayMode = sdlLayer.getPrimaryDisplayDesktopDisplayMode();
    if (desktopDisplayMode == nullptr)
    {
        priv::errMsg("`getDesktopMode` failed, returning default video mode");
        return {};
    }

    return sdlLayer.getVideoModeFromSDLDisplayMode(*desktopDisplayMode);
}


////////////////////////////////////////////////////////////
zb::Span<const VideoMode> VideoModeUtils::getFullscreenModes()
{
    static const auto cachedModes = []
    {
        zb::Vector<VideoMode> result;

        auto& sdlLayer = WindowContext::getSDLLayer();

        const auto displays = sdlLayer.getDisplays();
        if (!displays.valid() || displays.size() == 0)
            return result;

        const auto displayId = displays[0];
        const auto modes     = sdlLayer.getFullscreenDisplayModesForDisplay(displayId);

        if (!modes.valid())
            return result;

        for (const auto* mode : modes)
        {
            ZB_ASSERT(mode != nullptr);

            const za::VideoMode res = sdlLayer.getVideoModeFromSDLDisplayMode(*mode);

            if (zb::find(result.begin(), result.end(), res) == result.end())
                result.pushBack(res);
        }

        zb::quickSort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return {cachedModes.data(), cachedModes.size()};
}

} // namespace za
