// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/VideoMode.hpp"

#include "Zancle/Window/VideoModeUtils.hpp"

#include "Zancle/Algorithm/Find.hpp"

#include "Zancle/Vocabulary/Span.hpp"


namespace za
{
////////////////////////////////////////////////////////////
bool VideoMode::isValid() const
{
    const za::Span<const VideoMode> modes = VideoModeUtils::getFullscreenModes();

    return za::find(modes.begin(), modes.end(), *this) != modes.end();
}


} // namespace za
