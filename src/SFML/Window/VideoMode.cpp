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
    if (lhs.bitsPerPixel == rhs.bitsPerPixel)
    {
        if (lhs.size.x == rhs.size.x)
            return lhs.size.y < rhs.size.y;

        return lhs.size.x < rhs.size.x;
    }

    return lhs.bitsPerPixel < rhs.bitsPerPixel;
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
