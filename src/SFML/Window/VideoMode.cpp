#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/VideoModeImpl.hpp>

#include <SFML/Base/Algorithm.hpp>

#include <algorithm>


namespace sf
{
////////////////////////////////////////////////////////////
VideoMode::VideoMode(Vector2u modeSize, unsigned int modeBitsPerPixel) : size(modeSize), bitsPerPixel(modeBitsPerPixel)
{
}


////////////////////////////////////////////////////////////
VideoMode VideoMode::getDesktopMode()
{
    // Directly forward to the OS-specific implementation
    return priv::VideoModeImpl::getDesktopMode();
}


////////////////////////////////////////////////////////////
const std::vector<VideoMode>& VideoMode::getFullscreenModes()
{
    static const auto modes = []
    {
        std::vector<VideoMode> result = priv::VideoModeImpl::getFullscreenModes();
        std::sort(result.begin(), result.end(), [](const auto& lhs, const auto& rhs) { return lhs > rhs; });
        return result;
    }();

    return modes;
}


////////////////////////////////////////////////////////////
bool VideoMode::isValid() const
{
    const std::vector<VideoMode>& modes = getFullscreenModes();

    return base::find(modes.begin(), modes.end(), *this) != modes.end();
}


////////////////////////////////////////////////////////////
bool operator==(const VideoMode& left, const VideoMode& right)
{
    return (left.size == right.size) && (left.bitsPerPixel == right.bitsPerPixel);
}


////////////////////////////////////////////////////////////
bool operator!=(const VideoMode& left, const VideoMode& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
bool operator<(const VideoMode& left, const VideoMode& right)
{
    if (left.bitsPerPixel == right.bitsPerPixel)
    {
        if (left.size.x == right.size.x)
        {
            return left.size.y < right.size.y;
        }

        return left.size.x < right.size.x;
    }

    return left.bitsPerPixel < right.bitsPerPixel;
}


////////////////////////////////////////////////////////////
bool operator>(const VideoMode& left, const VideoMode& right)
{
    return right < left;
}


////////////////////////////////////////////////////////////
bool operator<=(const VideoMode& left, const VideoMode& right)
{
    return !(right < left);
}


////////////////////////////////////////////////////////////
bool operator>=(const VideoMode& left, const VideoMode& right)
{
    return !(left < right);
}

} // namespace sf
