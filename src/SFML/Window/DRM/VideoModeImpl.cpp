#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/DRM/DRMContext.hpp"
#include "SFML/Window/VideoModeImpl.hpp"

#include "SFML/System/Err.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    if (const auto* conn = DRMContext::getDRM().savedConnector)
    {
        for (int i = 0; i < conn->count_modes; i++)
            modes.push_back(VideoMode({conn->modes[i].hdisplay, conn->modes[i].vdisplay}));
    }
    else
        modes.push_back(getDesktopMode());

    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    const Drm&               drm = DRMContext::getDRM();
    const drmModeModeInfoPtr ptr = drm.mode ? drm.mode : &drm.originalCrtc->mode;
    return VideoMode({ptr->hdisplay, ptr->vdisplay});
}

} // namespace sf::priv
