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
#include <SFML/Audio/Listener.hpp>
#include <SFML/Audio/SavedSettings.hpp>

#include <SFML/System/Vector3.hpp>

#include <miniaudio.h>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] sf::Vector3f convertMAVec3(const ma_vec3f& v)
{
    return {v.x, v.y, v.z};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::pure]] sf::Listener::Cone convertMACone(const ma_sound& sound)
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


namespace sf::priv
{
////////////////////////////////////////////////////////////
SavedSettings::SavedSettings(const ma_sound& sound) :
pitch{ma_sound_get_pitch(&sound)},
pan{ma_sound_get_pan(&sound)},
volume{ma_sound_get_volume(&sound)},
spatializationEnabled{static_cast<bool>(ma_sound_is_spatialization_enabled(&sound))},
position{convertMAVec3(ma_sound_get_position(&sound))},
direction{convertMAVec3(ma_sound_get_direction(&sound))},
directionalAttenuationFactor{ma_sound_get_directional_attenuation_factor(&sound)},
velocity{convertMAVec3(ma_sound_get_velocity(&sound))},
dopplerFactor{ma_sound_get_doppler_factor(&sound)},
positioning{ma_sound_get_positioning(&sound)},
minDistance{ma_sound_get_min_distance(&sound)},
maxDistance{ma_sound_get_max_distance(&sound)},
minGain{ma_sound_get_min_gain(&sound)},
maxGain{ma_sound_get_max_gain(&sound)},
rollOff{ma_sound_get_rolloff(&sound)},
playing{static_cast<bool>(ma_sound_is_playing(&sound))},
looping{static_cast<bool>(ma_sound_is_looping(&sound))},
cone{convertMACone(sound)}
{
}


////////////////////////////////////////////////////////////
void SavedSettings::applyOnto(ma_sound& sound) const
{
    ma_sound_set_pitch(&sound, pitch);
    ma_sound_set_pan(&sound, pan);
    ma_sound_set_volume(&sound, volume);
    ma_sound_set_spatialization_enabled(&sound, spatializationEnabled);
    ma_sound_set_position(&sound, position.x, position.y, position.z);
    ma_sound_set_direction(&sound, direction.x, direction.y, direction.z);
    ma_sound_set_directional_attenuation_factor(&sound, directionalAttenuationFactor);
    ma_sound_set_velocity(&sound, velocity.x, velocity.y, velocity.z);
    ma_sound_set_doppler_factor(&sound, dopplerFactor);
    ma_sound_set_positioning(&sound, static_cast<ma_positioning>(positioning));
    ma_sound_set_min_distance(&sound, minDistance);
    ma_sound_set_max_distance(&sound, maxDistance);
    ma_sound_set_min_gain(&sound, minGain);
    ma_sound_set_max_gain(&sound, maxGain);
    ma_sound_set_rolloff(&sound, rollOff);
    ma_sound_set_looping(&sound, looping);
    ma_sound_set_cone(&sound, cone.innerAngle.asRadians(), cone.outerAngle.asRadians(), cone.outerGain);

    if (playing)
        ma_sound_start(&sound);
    else
        ma_sound_stop(&sound);
}


} // namespace sf::priv
