// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/NativeActivity.hpp"

#include "Zancle/System/Android/Activity.hpp"


namespace za
{
////////////////////////////////////////////////////////////
ANativeActivity* getNativeActivity()
{
    return priv::getActivity().activity;
}

} // namespace za
