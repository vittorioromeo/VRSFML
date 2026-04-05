// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/VideoMode.hpp"

#include "SFML/Window/VideoModeUtils.hpp"

#include "SFML/Base/Algorithm/Find.hpp"
#include "SFML/Base/Span.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
bool VideoMode::isValid() const
{
    const base::Span<const VideoMode> modes = VideoModeUtils::getFullscreenModes();

    return base::find(modes.begin(), modes.end(), *this) != modes.end();
}


} // namespace sf
