////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
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
#include <SFML/System/TimeChronoUtil.hpp> // NOLINT(misc-header-include-cycle)

#include <chrono>

#include <cassert>


namespace sf
{
////////////////////////////////////////////////////////////
template <typename Rep, typename Period>
constexpr Time TimeChronoUtil::fromDuration(const std::chrono::duration<Rep, Period>& duration)
{
    return Time(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
}

////////////////////////////////////////////////////////////
constexpr std::chrono::microseconds TimeChronoUtil::toDuration(Time time)
{
    return std::chrono::microseconds(time.asMicroseconds());
}


////////////////////////////////////////////////////////////
template <typename Rep, typename Period>
constexpr std::chrono::duration<Rep, Period> TimeChronoUtil::toCustomDuration(Time time)
{
    return std::chrono::microseconds(time.asMicroseconds());
}

} // namespace sf