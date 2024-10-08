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

    const Drm&          drm  = DRMContext::getDRM();
    drmModeConnectorPtr conn = drm.savedConnector;

    if (conn)
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
    const Drm&         drm = DRMContext::getDRM();
    drmModeModeInfoPtr ptr = drm.mode;
    if (ptr != nullptr)
        return VideoMode({ptr->hdisplay, ptr->vdisplay});

    return VideoMode({0, 0});
}

} // namespace sf::priv
