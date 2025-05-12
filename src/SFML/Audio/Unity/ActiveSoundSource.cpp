#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ActiveSoundSource.hpp"
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/EffectProcessor.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Clamp.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct ActiveSoundSource::Impl
{
    AudioSettings   audioSettings;
    EffectProcessor effectProcessor{};
};


////////////////////////////////////////////////////////////
ActiveSoundSource::ActiveSoundSource() = default;


////////////////////////////////////////////////////////////
ActiveSoundSource::ActiveSoundSource(const ActiveSoundSource&) = default;


////////////////////////////////////////////////////////////
ActiveSoundSource::ActiveSoundSource(ActiveSoundSource&&) noexcept = default;


////////////////////////////////////////////////////////////
ActiveSoundSource& ActiveSoundSource::operator=(const ActiveSoundSource& rhs) = default;


////////////////////////////////////////////////////////////
ActiveSoundSource& ActiveSoundSource::operator=(ActiveSoundSource&&) noexcept = default;


////////////////////////////////////////////////////////////
ActiveSoundSource::~ActiveSoundSource() = default;


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPitch(const float pitch)
{
    m_impl->audioSettings.pitch = pitch;
    ma_sound_set_pitch(&getSound(), pitch);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPan(const float pan)
{
    m_impl->audioSettings.pan = pan;
    ma_sound_set_pan(&getSound(), pan);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setVolume(const float volume)
{
    SFML_BASE_ASSERT(volume >= 0.f && volume <= 1.f);

    m_impl->audioSettings.volume = volume;
    ma_sound_set_volume(&getSound(), volume);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setSpatializationEnabled(bool spatializationEnabled)
{
    m_impl->audioSettings.spatializationEnabled = spatializationEnabled;
    ma_sound_set_spatialization_enabled(&getSound(), spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPosition(const Vec3f& position)
{
    m_impl->audioSettings.position = position;
    ma_sound_set_position(&getSound(), position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDirection(const Vec3f& direction)
{
    m_impl->audioSettings.direction = direction;
    ma_sound_set_direction(&getSound(), direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setCone(const Cone& cone)
{
    m_impl->audioSettings.cone = cone;

    ma_sound_set_cone(&getSound(),
                      base::clamp(m_impl->audioSettings.cone.innerAngle, Angle::Zero, Angle::Full).asRadians(),
                      base::clamp(m_impl->audioSettings.cone.outerAngle, Angle::Zero, Angle::Full).asRadians(),
                      m_impl->audioSettings.cone.outerGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setVelocity(const Vec3f& velocity)
{
    m_impl->audioSettings.velocity = velocity;
    ma_sound_set_velocity(&getSound(), velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDopplerFactor(const float dopplerFactor)
{
    m_impl->audioSettings.dopplerFactor = dopplerFactor;
    ma_sound_set_doppler_factor(&getSound(), dopplerFactor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDirectionalAttenuationFactor(const float directionalAttenuationFactor)
{
    m_impl->audioSettings.directionalAttenuationFactor = directionalAttenuationFactor;
    ma_sound_set_directional_attenuation_factor(&getSound(), directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setRelativeToListener(bool relativeToListener)
{
    m_impl->audioSettings.positioning = relativeToListener ? ma_positioning_relative : ma_positioning_absolute;
    ma_sound_set_positioning(&getSound(), relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMinDistance(const float minDistance)
{
    m_impl->audioSettings.minDistance = minDistance;
    ma_sound_set_min_distance(&getSound(), minDistance);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMaxDistance(const float maxDistance)
{
    m_impl->audioSettings.maxDistance = maxDistance;
    ma_sound_set_max_distance(&getSound(), maxDistance);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMinGain(const float minGain)
{
    m_impl->audioSettings.minGain = minGain;
    ma_sound_set_min_gain(&getSound(), minGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMaxGain(const float maxGain)
{
    m_impl->audioSettings.maxGain = maxGain;
    ma_sound_set_max_gain(&getSound(), maxGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setAttenuation(const float attenuation)
{
    m_impl->audioSettings.rollOff = attenuation;
    ma_sound_set_rolloff(&getSound(), attenuation);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    m_impl->effectProcessor = effectProcessor;
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setLooping(bool loop)
{
    m_impl->audioSettings.looping = loop;
    ma_sound_set_looping(&getSound(), loop ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getPitch() const
{
    return m_impl->audioSettings.pitch;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getPan() const
{
    return m_impl->audioSettings.pan;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getVolume() const
{
    const float result = m_impl->audioSettings.volume;
    SFML_BASE_ASSERT(result >= 0.f && result <= 1.f);
    return result;
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::isSpatializationEnabled() const
{
    return m_impl->audioSettings.spatializationEnabled;
}


////////////////////////////////////////////////////////////
Vec3f ActiveSoundSource::getPosition() const
{
    return m_impl->audioSettings.position;
}


////////////////////////////////////////////////////////////
Vec3f ActiveSoundSource::getDirection() const
{
    return m_impl->audioSettings.direction;
}


////////////////////////////////////////////////////////////
ActiveSoundSource::Cone ActiveSoundSource::getCone() const
{
    return m_impl->audioSettings.cone;
}


////////////////////////////////////////////////////////////
Vec3f ActiveSoundSource::getVelocity() const
{
    return m_impl->audioSettings.velocity;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getDopplerFactor() const
{
    return m_impl->audioSettings.dopplerFactor;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getDirectionalAttenuationFactor() const
{
    return m_impl->audioSettings.directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::isRelativeToListener() const
{
    return m_impl->audioSettings.positioning == ma_positioning_relative;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getMinDistance() const
{
    return m_impl->audioSettings.minDistance;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getMaxDistance() const
{
    return m_impl->audioSettings.maxDistance;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getMinGain() const
{
    return m_impl->audioSettings.minGain;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getMaxGain() const
{
    return m_impl->audioSettings.maxGain;
}


////////////////////////////////////////////////////////////
float ActiveSoundSource::getAttenuation() const
{
    return m_impl->audioSettings.rollOff;
}


////////////////////////////////////////////////////////////
const EffectProcessor& ActiveSoundSource::getEffectProcessor() const
{
    return m_impl->effectProcessor;
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::isLooping() const
{
    return m_impl->audioSettings.looping;
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::applyAudioSettings(ma_sound& sound) const
{
    const auto& s = m_impl->audioSettings;

    ma_sound_set_cone(&sound, s.cone.innerAngle.asRadians(), s.cone.outerAngle.asRadians(), s.cone.outerGain);
    ma_sound_set_position(&sound, s.position.x, s.position.y, s.position.z);
    ma_sound_set_direction(&sound, s.direction.x, s.direction.y, s.direction.z);
    ma_sound_set_velocity(&sound, s.velocity.x, s.velocity.y, s.velocity.z);
    ma_sound_set_pitch(&sound, s.pitch);
    ma_sound_set_pan(&sound, s.pan);
    ma_sound_set_volume(&sound, s.volume);
    ma_sound_set_directional_attenuation_factor(&sound, s.directionalAttenuationFactor);
    ma_sound_set_doppler_factor(&sound, s.dopplerFactor);
    ma_sound_set_min_distance(&sound, s.minDistance);
    ma_sound_set_max_distance(&sound, s.maxDistance);
    ma_sound_set_min_gain(&sound, s.minGain);
    ma_sound_set_max_gain(&sound, s.maxGain);
    ma_sound_set_rolloff(&sound, s.rollOff);
    ma_sound_set_positioning(&sound, static_cast<ma_positioning>(s.positioning));
    ma_sound_set_looping(&sound, s.looping);
    ma_sound_set_spatialization_enabled(&sound, s.spatializationEnabled);

    /*
    if (s.playing)
        ma_sound_start(&sound);
    else
        ma_sound_stop(&sound);
    */
}

} // namespace sf
