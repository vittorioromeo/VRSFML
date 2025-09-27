// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioSettings.hpp"

#include "SFML/Audio/Listener.hpp"

#include "SFML/System/Vec3.hpp"

#include <miniaudio.h>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Vec3f convertMAVec3(const ma_vec3f& v)
{
    return {v.x, v.y, v.z};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline sf::Listener::Cone convertMACone(const ma_sound& sound)
{
    sf::Listener::Cone cone;

    float innerAngle{};
    float outerAngle{};
    ma_sound_get_cone(&sound, &innerAngle, &outerAngle, &cone.outerGain);

    cone.innerAngle = sf::radians(innerAngle);
    cone.outerAngle = sf::radians(outerAngle);

    return cone;
}

} // namespace
