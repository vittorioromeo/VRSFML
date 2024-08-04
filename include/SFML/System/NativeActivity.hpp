#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>


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
/// You shouldn't have to use this function, unless you want
/// to implement very specific details, that SFML doesn't
/// support, or to use a workaround for a known issue.
///
/// \return Pointer to Android native activity structure
///
/// \sfplatform{Android,SFML/System/NativeActivity.hpp}
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API ANativeActivity* getNativeActivity();

} // namespace sf
