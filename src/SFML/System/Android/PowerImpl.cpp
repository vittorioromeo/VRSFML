////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2014 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Android/PowerImpl.hpp>
#include <SFML/System/Android/Activity.hpp>
#include <SFML/System/Lock.hpp>
#include <android/native_activity.h>
#include <android/window.h>

namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
bool setPowersavingEnabledImpl(bool enabled)
{
    ActivityStates* states = getActivity(NULL);
    Lock(states->mutex);
	if (enabled)
	{
		ANativeActivity_setWindowFlags(states->activity, 0, AWINDOW_FLAG_KEEP_SCREEN_ON);
	}
	else
	{
		ANativeActivity_setWindowFlags(states->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
	}
	return false;
}

} // namespace priv

} // namespace sf
