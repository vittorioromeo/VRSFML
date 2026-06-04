// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/AudioSettings.hpp"

#include "Zancle/Audio/Listener.hpp"

#include "Zancle/System/Vec3.hpp"

#include <miniaudio.h>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline za::Vec3f convertMAVec3(const ma_vec3f& v)
{
    return {v.x, v.y, v.z};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] inline za::Listener::Cone convertMACone(const ma_sound& sound)
{
    za::Listener::Cone cone;

    float innerAngle{};
    float outerAngle{};
    ma_sound_get_cone(&sound, &innerAngle, &outerAngle, &cone.outerGain);

    cone.innerAngle = za::radians(innerAngle);
    cone.outerAngle = za::radians(outerAngle);

    return cone;
}

} // namespace
