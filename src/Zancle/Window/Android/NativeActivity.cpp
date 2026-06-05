// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/NativeActivity.hpp"

#include "Zancle/Window/Android/Activity.hpp"


namespace za
{
////////////////////////////////////////////////////////////
ANativeActivity* getNativeActivity()
{
    return priv::getActivity().activity;
}

} // namespace za
