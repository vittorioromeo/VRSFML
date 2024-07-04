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
#include <SFML/Audio/EffectProcessor.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundSource.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Macros.hpp>

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
void SoundSource::setPitch(float pitch)
{
    m_pitch = pitch;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_pitch(sound, m_pitch);
}


////////////////////////////////////////////////////////////
void SoundSource::setPan(float pan)
{
    m_pan = pan;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_pan(sound, m_pan);
}


////////////////////////////////////////////////////////////
void SoundSource::setVolume(float volume)
{
    m_volume = volume;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_volume(sound, m_volume * 0.01f);
}


////////////////////////////////////////////////////////////
void SoundSource::setSpatializationEnabled(bool spatializationEnabled)
{
    m_spatializationEnabled = spatializationEnabled;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_spatialization_enabled(sound, m_spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void SoundSource::setPosition(const Vector3f& position)
{
    m_position = position;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_position(sound, m_position.x, m_position.y, m_position.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setDirection(const Vector3f& direction)
{
    m_direction = direction;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_direction(sound, m_direction.x, m_direction.y, m_direction.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setCone(const Cone& cone)
{
    m_cone = cone;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_cone(sound,
                          priv::clamp(m_cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                          priv::clamp(m_cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                          m_cone.outerGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setVelocity(const Vector3f& velocity)
{
    m_velocity = velocity;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_velocity(sound, m_velocity.x, m_velocity.y, m_velocity.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setDopplerFactor(float dopplerFactor)
{
    m_dopplerFactor = dopplerFactor;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_doppler_factor(sound, m_dopplerFactor);
}


////////////////////////////////////////////////////////////
void SoundSource::setDirectionalAttenuationFactor(float directionalAttenuationFactor)
{
    m_directionalAttenuationFactor = directionalAttenuationFactor;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_directional_attenuation_factor(sound, m_directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void SoundSource::setRelativeToListener(bool relativeToListener)
{
    m_relativeToListener = relativeToListener;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_positioning(sound, m_relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void SoundSource::setMinDistance(float minDistance)
{
    m_minDistance = minDistance;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_min_distance(sound, m_minDistance);
}


////////////////////////////////////////////////////////////
void SoundSource::setMaxDistance(float maxDistance)
{
    m_maxDistance = maxDistance;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_max_distance(sound, m_maxDistance);
}


////////////////////////////////////////////////////////////
void SoundSource::setMinGain(float minGain)
{
    m_minGain = minGain;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_min_gain(sound, m_minGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setMaxGain(float maxGain)
{
    m_maxGain = maxGain;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_max_gain(sound, m_maxGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setAttenuation(float attenuation)
{
    m_attenuation = attenuation;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_rolloff(sound, m_attenuation);
}


////////////////////////////////////////////////////////////
void SoundSource::setEffectProcessor(EffectProcessor effectProcessor)
{
    m_effectProcessor = SFML_MOVE(effectProcessor);
}


////////////////////////////////////////////////////////////
void SoundSource::setLoop(bool loop)
{
    m_loop = loop;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void SoundSource::setPlayingOffset(Time playingOffset)
{
    m_playingOffset = playingOffset;
}


////////////////////////////////////////////////////////////
float SoundSource::getPitch() const
{
    return 0.f;
}


////////////////////////////////////////////////////////////
float SoundSource::getPan() const
{
    return m_pan;
}


////////////////////////////////////////////////////////////
float SoundSource::getVolume() const
{
    return m_volume;
}


////////////////////////////////////////////////////////////
bool SoundSource::isSpatializationEnabled() const
{
    return m_spatializationEnabled;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getPosition() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_position;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getDirection() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_direction;
}


////////////////////////////////////////////////////////////
SoundSource::Cone SoundSource::getCone() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_cone;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getVelocity() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_velocity;
}


////////////////////////////////////////////////////////////
float SoundSource::getDopplerFactor() const
{
    return m_dopplerFactor;
}


////////////////////////////////////////////////////////////
float SoundSource::getDirectionalAttenuationFactor() const
{
    return m_directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
bool SoundSource::isRelativeToListener() const
{
    return m_relativeToListener;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinDistance() const
{
    return m_minDistance;
}


////////////////////////////////////////////////////////////
float SoundSource::getMaxDistance() const
{
    return m_maxDistance;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinGain() const
{
    return m_minGain;
}


////////////////////////////////////////////////////////////
float SoundSource::getMaxGain() const
{
    return m_maxGain;
}


////////////////////////////////////////////////////////////
float SoundSource::getAttenuation() const
{
    return m_attenuation;
}


////////////////////////////////////////////////////////////
EffectProcessor SoundSource::getEffectProcessor() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_effectProcessor;
}


////////////////////////////////////////////////////////////
bool SoundSource::getLoop() const
{
    return m_loop;
}


////////////////////////////////////////////////////////////
Time SoundSource::getPlayingOffset() const
{
    return m_playingOffset;
}


////////////////////////////////////////////////////////////
SoundSource& SoundSource::operator=(const SoundSource& right)
{
    // Assign the sound attributes
    setPitch(right.getPitch());
    setPan(right.getPan());
    setVolume(right.getVolume());
    setSpatializationEnabled(right.isSpatializationEnabled());
    setPosition(right.getPosition());
    setDirection(right.getDirection());
    setCone(right.getCone());
    setVelocity(right.getVelocity());
    setDopplerFactor(right.getDopplerFactor());
    setRelativeToListener(right.isRelativeToListener());
    setMinDistance(right.getMinDistance());
    setMaxDistance(right.getMaxDistance());
    setMinGain(right.getMinGain());
    setMaxGain(right.getMaxGain());
    setAttenuation(right.getAttenuation());
    setEffectProcessor(right.getEffectProcessor());
    setLoop(right.getLoop());
    setPlayingOffset(right.getPlayingOffset());

    return *this;
}

void SoundSource::applyStoredSettings(void* soundPtr) const
{
    auto* sound = static_cast<ma_sound*>(soundPtr);
    assert(sound != nullptr);

    ma_sound_set_pitch(sound, m_pitch);
    ma_sound_set_pan(sound, m_pan);
    ma_sound_set_volume(sound, m_volume * 0.01f);
    ma_sound_set_spatialization_enabled(sound, m_spatializationEnabled ? MA_TRUE : MA_FALSE);
    ma_sound_set_position(sound, m_position.x, m_position.y, m_position.z);
    ma_sound_set_direction(sound, m_direction.x, m_direction.y, m_direction.z);
    ma_sound_set_cone(sound,
                      priv::clamp(m_cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                      priv::clamp(m_cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                      m_cone.outerGain);
    ma_sound_set_velocity(sound, m_velocity.x, m_velocity.y, m_velocity.z);
    ma_sound_set_doppler_factor(sound, m_dopplerFactor);
    ma_sound_set_directional_attenuation_factor(sound, m_directionalAttenuationFactor);
    ma_sound_set_positioning(sound, m_relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
    ma_sound_set_min_distance(sound, m_minDistance);
    ma_sound_set_max_distance(sound, m_maxDistance);
    ma_sound_set_min_gain(sound, m_minGain);
    ma_sound_set_max_gain(sound, m_maxGain);
    ma_sound_set_rolloff(sound, m_attenuation);
    ma_sound_set_looping(sound, m_loop ? MA_TRUE : MA_FALSE);
}


} // namespace sf
