#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeImpl.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <algorithm> // std::sort


namespace sf
{
////////////////////////////////////////////////////////////
VideoMode VideoModeUtils::getDesktopMode()
{
    // Directly forward to the OS-specific implementation
    return priv::VideoModeImpl::getDesktopMode();
}


////////////////////////////////////////////////////////////
base::Span<const VideoMode> VideoModeUtils::getFullscreenModes()
{
    static const auto modes = []
    {
        std::vector<VideoMode> result;

        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        }

        int            display_count = 0;
        SDL_DisplayID* displays      = SDL_GetDisplays(&display_count);
        if (displays == nullptr)
        {
            SDL_Log("SDL_GetDisplays failed: %s", SDL_GetError());
        }

        for (int i = 0; i < display_count; ++i)
        {
            SDL_DisplayID     display_id = displays[i];
            int               mode_count = 0;
            SDL_DisplayMode** modes      = SDL_GetFullscreenDisplayModes(display_id, &mode_count);
            if (modes == nullptr)
            {
                SDL_Log("SDL_GetFullscreenDisplayModes failed for display %d: %s", display_id, SDL_GetError());
                continue;
            }

            SDL_Log("Display %d has %d fullscreen modes:", display_id, mode_count);
            for (int j = 0; j < mode_count; ++j)
            {
                const SDL_DisplayMode* mode = modes[j];

                // Retrieve pixel format info
                SDL_PixelFormat               format = mode->format;
                const SDL_PixelFormatDetails* info   = SDL_GetPixelFormatDetails(format);
                if (!info)
                {
                    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Failed to get pixel format info for format %d", format);
                }

                int bpp = info->bits_per_pixel;

                sf::VideoMode res{{static_cast<unsigned int>(mode->w), static_cast<unsigned int>(mode->h)},
                                  static_cast<unsigned int>(bpp)};

                if (std::find(result.begin(), result.end(), res) == result.end())
                    result.push_back(res);

                //                SDL_Log("  Mode %d: %dx%d @ %.2fHz", j, mode->w, mode->h, mode->refresh_rate);
            }
        }


        SDL_free(displays);

        std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return {modes.data(), modes.size()};
}

} // namespace sf
