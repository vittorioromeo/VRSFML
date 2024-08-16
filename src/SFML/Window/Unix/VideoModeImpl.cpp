#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Unix/Display.hpp>
#include <SFML/Window/Unix/Utils.hpp>
#include <SFML/Window/VideoModeImpl.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Algorithm.hpp>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#include <utility>


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <>
struct XDeleter<XRRScreenConfiguration>
{
    void operator()(XRRScreenConfiguration* config) const
    {
        XRRFreeScreenConfigInfo(config);
    }
};


////////////////////////////////////////////////////////////
std::vector<VideoMode> VideoModeImpl::getFullscreenModes()
{
    std::vector<VideoMode> modes;

    // Open a connection with the X server
    if (const auto display = openDisplay())
    {
        // Retrieve the default screen number
        const int screen = DefaultScreen(display.get());

        // Check if the XRandR extension is present
        int version = 0;
        if (XQueryExtension(display.get(), "RANDR", &version, &version, &version))
        {
            // Get the current configuration
            const auto config = X11Ptr<XRRScreenConfiguration>(
                XRRGetScreenInfo(display.get(), RootWindow(display.get(), screen)));
            if (config)
            {
                // Get the available screen sizes
                int            nbSizes = 0;
                XRRScreenSize* sizes   = XRRConfigSizes(config.get(), &nbSizes);
                if (sizes && (nbSizes > 0))
                {
                    // Get the list of supported depths
                    int        nbDepths = 0;
                    const auto depths   = X11Ptr<int[]>(XListDepths(display.get(), screen, &nbDepths));
                    if (depths && (nbDepths > 0))
                    {
                        // Combine depths and sizes to fill the array of supported modes
                        for (std::size_t i = 0; i < static_cast<std::size_t>(nbDepths); ++i)
                        {
                            for (int j = 0; j < nbSizes; ++j)
                            {
                                // Convert to VideoMode
                                VideoMode mode({static_cast<unsigned int>(sizes[j].width),
                                                static_cast<unsigned int>(sizes[j].height)},
                                               static_cast<unsigned int>(depths.get()[i]));

                                Rotation currentRotation = 0;
                                XRRConfigRotations(config.get(), &currentRotation);

                                if (currentRotation == RR_Rotate_90 || currentRotation == RR_Rotate_270)
                                    std::swap(mode.size.x, mode.size.y);

                                // Add it only if it is not already in the array
                                if (base::find(modes.begin(), modes.end(), mode) == modes.end())
                                    modes.push_back(mode);
                            }
                        }
                    }
                }
            }
            else
            {
                // Failed to get the screen configuration
                priv::err() << "Failed to retrieve the screen configuration while trying to get the supported video "
                               "modes";
            }
        }
        else
        {
            // XRandr extension is not supported: we cannot get the video modes
            priv::err() << "Failed to use the XRandR extension while trying to get the supported video modes";
        }
    }
    else
    {
        // We couldn't connect to the X server
        priv::err() << "Failed to connect to the X server while trying to get the supported video modes";
    }

    return modes;
}


////////////////////////////////////////////////////////////
VideoMode VideoModeImpl::getDesktopMode()
{
    VideoMode desktopMode{{0u, 0u}, 0u}; // TODO P1: avoid null state

    // Open a connection with the X server
    const std::shared_ptr<Display> display = openDisplay();
    if (display == nullptr)
    {
        // We couldn't connect to the X server
        priv::err() << "Failed to connect to the X server while trying to get the desktop video modes";
        return desktopMode;
    }

    // Retrieve the default screen number
    const int screen = DefaultScreen(display.get());

    // Check if the XRandR extension is present
    int version = 0;
    if (!XQueryExtension(display.get(), "RANDR", &version, &version, &version))
    {
        // XRandr extension is not supported: we cannot get the video modes
        priv::err() << "Failed to use the XRandR extension while trying to get the desktop video modes";
        return desktopMode;
    }

    // Get the current configuration
    const auto config = X11Ptr<XRRScreenConfiguration>(XRRGetScreenInfo(display.get(), RootWindow(display.get(), screen)));
    if (config == nullptr)
    {
        // Failed to get the screen configuration
        priv::err() << "Failed to retrieve the screen configuration while trying to get the desktop video modes";
        return desktopMode;
    }

    // Get the current video mode
    Rotation  currentRotation = 0;
    const int currentMode     = XRRConfigCurrentConfiguration(config.get(), &currentRotation);

    // Get the available screen sizes
    int            nbSizes = 0;
    XRRScreenSize* sizes   = XRRConfigSizes(config.get(), &nbSizes);
    if (sizes && (nbSizes > 0))
    {
        desktopMode = VideoMode({static_cast<unsigned int>(sizes[currentMode].width),
                                 static_cast<unsigned int>(sizes[currentMode].height)},
                                static_cast<unsigned int>(DefaultDepth(display.get(), screen)));

        Rotation modeRotation = 0;
        XRRConfigRotations(config.get(), &modeRotation);

        if (modeRotation == RR_Rotate_90 || modeRotation == RR_Rotate_270)
            std::swap(desktopMode.size.x, desktopMode.size.y);
    }

    return desktopMode;
}

} // namespace sf::priv
