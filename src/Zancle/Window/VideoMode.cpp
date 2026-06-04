// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/VideoMode.hpp"

#include "Zancle/Window/VideoModeUtils.hpp"

#include "ZancleBase/Algorithm/Find.hpp"
#include "ZancleBase/Span.hpp"


namespace za
{
////////////////////////////////////////////////////////////
bool VideoMode::isValid() const
{
    const base::Span<const VideoMode> modes = VideoModeUtils::getFullscreenModes();

    return base::find(modes.begin(), modes.end(), *this) != modes.end();
}


} // namespace za
