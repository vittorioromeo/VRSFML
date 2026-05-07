#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Export.hpp"


#if !defined(SFML_SYSTEM_ANDROID)
    #error NativeActivity.hpp: This header is Android only.
#endif


struct ANativeActivity;


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup system
/// \brief Return a pointer to the Android native activity
///
/// Escape hatch for platform-specific details that SFML doesn't expose
/// or for working around known issues; rarely needed in regular use.
///
/// \sfplatform{Android,SFML/System/NativeActivity.hpp}
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API ANativeActivity* getNativeActivity();

} // namespace sf
