// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/VideoModeUtils.hpp"

#include "Zancle/Window/SDLLayer.hpp"
#include "Zancle/Window/VideoMode.hpp"
#include "Zancle/Window/WindowContext.hpp"

#include "Zancle/Err/Err.hpp"

#include "Zancle/Algorithm/Find.hpp"
#include "Zancle/Algorithm/Sort.hpp"
#include "Zancle/Diagnostic/Assert.hpp"
#include "Zancle/Vocabulary/Span.hpp"
#include "Zancle/Container/Vector.hpp"


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
za::Span<const VideoMode> VideoModeUtils::getFullscreenModes()
{
    static const auto cachedModes = []
    {
        za::Vector<VideoMode> result;

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
            ZA_ASSERT(mode != nullptr);

            const za::VideoMode res = sdlLayer.getVideoModeFromSDLDisplayMode(*mode);

            if (za::find(result.begin(), result.end(), res) == result.end())
                result.pushBack(res);
        }

        za::quickSort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return {cachedModes.data(), cachedModes.size()};
}

} // namespace za
