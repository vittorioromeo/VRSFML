// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/NativeActivity.hpp"

#include "SFML/System/Android/Activity.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
ANativeActivity* getNativeActivity()
{
    return priv::getActivity().activity;
}

} // namespace sf
