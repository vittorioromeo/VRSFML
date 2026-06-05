#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Config.hpp"


#if !defined(ZA_SYSTEM_ANDROID)
    #error NativeActivity.hpp: This header is Android only.
#endif


struct ANativeActivity;


namespace za
{
////////////////////////////////////////////////////////////
/// \ingroup system
/// \brief Return a pointer to the Android native activity
///
/// Escape hatch for platform-specific details that Zancle doesn't expose
/// or for working around known issues; rarely needed in regular use.
///
/// \sfplatform{Android,Zancle/Window/NativeActivity.hpp}
///
////////////////////////////////////////////////////////////
[[nodiscard]] ZA_SYSTEM_API ANativeActivity* getNativeActivity();

} // namespace za
