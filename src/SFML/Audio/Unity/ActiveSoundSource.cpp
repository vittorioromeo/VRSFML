#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ActiveSoundSource.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/InactiveSoundSource.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"

#include "SFML/Base/Clamp.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
void ActiveSoundSource::setPitch(const float pitch)
{
    InactiveSoundSource::setPitch(pitch);
    ma_sound_set_pitch(&getSoundBase().getSound(), pitch);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPan(const float pan)
{
    InactiveSoundSource::setPan(pan);
    ma_sound_set_pan(&getSoundBase().getSound(), pan);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setVolume(const float volume)
{
    SFML_BASE_ASSERT(volume >= 0.f && volume <= 1.f);

    InactiveSoundSource::setVolume(volume);
    ma_sound_set_volume(&getSoundBase().getSound(), volume);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setSpatializationEnabled(const bool spatializationEnabled)
{
    InactiveSoundSource::setSpatializationEnabled(spatializationEnabled);
    ma_sound_set_spatialization_enabled(&getSoundBase().getSound(), spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPosition(const Vec3f& position)
{
    InactiveSoundSource::setPosition(position);
    ma_sound_set_position(&getSoundBase().getSound(), position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDirection(const Vec3f& direction)
{
    InactiveSoundSource::setDirection(direction);
    ma_sound_set_direction(&getSoundBase().getSound(), direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setCone(const Cone& cone)
{
    InactiveSoundSource::setCone(cone);

    ma_sound_set_cone(&getSoundBase().getSound(),
                      base::clamp(cone.innerAngle, Angle::Zero, Angle::Full).asRadians(),
                      base::clamp(cone.outerAngle, Angle::Zero, Angle::Full).asRadians(),
                      cone.outerGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setVelocity(const Vec3f& velocity)
{
    InactiveSoundSource::setVelocity(velocity);
    ma_sound_set_velocity(&getSoundBase().getSound(), velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDopplerFactor(const float dopplerFactor)
{
    InactiveSoundSource::setDopplerFactor(dopplerFactor);
    ma_sound_set_doppler_factor(&getSoundBase().getSound(), dopplerFactor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDirectionalAttenuationFactor(const float directionalAttenuationFactor)
{
    InactiveSoundSource::setDirectionalAttenuationFactor(directionalAttenuationFactor);
    ma_sound_set_directional_attenuation_factor(&getSoundBase().getSound(), directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setRelativeToListener(const bool relativeToListener)
{
    InactiveSoundSource::setRelativeToListener(relativeToListener);
    ma_sound_set_positioning(&getSoundBase().getSound(),
                             relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMinDistance(const float minDistance)
{
    InactiveSoundSource::setMinDistance(minDistance);
    ma_sound_set_min_distance(&getSoundBase().getSound(), minDistance);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMaxDistance(const float maxDistance)
{
    InactiveSoundSource::setMaxDistance(maxDistance);
    ma_sound_set_max_distance(&getSoundBase().getSound(), maxDistance);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMinGain(const float minGain)
{
    InactiveSoundSource::setMinGain(minGain);
    ma_sound_set_min_gain(&getSoundBase().getSound(), minGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMaxGain(const float maxGain)
{
    InactiveSoundSource::setMaxGain(maxGain);
    ma_sound_set_max_gain(&getSoundBase().getSound(), maxGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setAttenuation(const float attenuation)
{
    InactiveSoundSource::setAttenuation(attenuation);
    ma_sound_set_rolloff(&getSoundBase().getSound(), attenuation);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    InactiveSoundSource::setEffectProcessor(effectProcessor);
    getSoundBase().setAndConnectEffectProcessor(effectProcessor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setLooping(const bool loop)
{
    InactiveSoundSource::setLooping(loop);
    ma_sound_set_looping(&getSoundBase().getSound(), loop ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::applySettings(const AudioSettings& audioSettings)
{
    getSoundBase().applyAudioSettings(audioSettings);
}

} // namespace sf
