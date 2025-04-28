#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoMode.hpp"
#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Span.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool VideoMode::isValid() const
{
    const base::Span<const VideoMode> modes = VideoModeUtils::getFullscreenModes();

    return base::find(modes.begin(), modes.end(), *this) != modes.end();
}


////////////////////////////////////////////////////////////
bool operator<(const VideoMode& lhs, const VideoMode& rhs)
{
    const auto vecLessThan = [](const Vector2u& lhs, const Vector2u& rhs) -> bool
    {
        if (lhs.x == rhs.x)
            return lhs.y < rhs.y;

        return lhs.x < rhs.x;
    };

    if (lhs.bitsPerPixel < rhs.bitsPerPixel)
        return true;

    if (rhs.bitsPerPixel < lhs.bitsPerPixel)
        return false;

    if (vecLessThan(lhs.size, rhs.size))
        return true;

    if (vecLessThan(rhs.size, lhs.size))
        return false;

    if (lhs.pixelDensity < rhs.pixelDensity)
        return true;

    if (rhs.pixelDensity < lhs.pixelDensity)
        return false;

    if (lhs.refreshRate < rhs.refreshRate)
        return true;

    return false;
}


////////////////////////////////////////////////////////////
bool operator>(const VideoMode& lhs, const VideoMode& rhs)
{
    return rhs < lhs;
}


////////////////////////////////////////////////////////////
bool operator<=(const VideoMode& lhs, const VideoMode& rhs)
{
    return !(rhs < lhs);
}


////////////////////////////////////////////////////////////
bool operator>=(const VideoMode& lhs, const VideoMode& rhs)
{
    return !(lhs < rhs);
}

} // namespace sf
