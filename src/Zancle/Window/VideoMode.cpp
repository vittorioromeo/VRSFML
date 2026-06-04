// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


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
    const zb::Span<const VideoMode> modes = VideoModeUtils::getFullscreenModes();

    return zb::find(modes.begin(), modes.end(), *this) != modes.end();
}


} // namespace za
