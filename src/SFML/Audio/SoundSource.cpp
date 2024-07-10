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
#include <SFML/Audio/SavedSettings.hpp>
#include <SFML/Audio/SoundSource.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/Time.hpp>

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundSource::Impl
{
    priv::SavedSettings savedSettings;
    EffectProcessor     effectProcessor{};
    Time                playingOffset;
};

////////////////////////////////////////////////////////////
SoundSource::SoundSource() = default;


////////////////////////////////////////////////////////////
SoundSource::SoundSource(const SoundSource&) = default;


////////////////////////////////////////////////////////////
SoundSource::SoundSource(SoundSource&&) noexcept = default;


////////////////////////////////////////////////////////////
SoundSource& SoundSource::operator=(SoundSource&&) noexcept = default;


////////////////////////////////////////////////////////////
SoundSource::~SoundSource() = default;


////////////////////////////////////////////////////////////
void SoundSource::setPitch(float pitch)
{
    m_impl->savedSettings.pitch = pitch;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_pitch(sound, pitch);
}


////////////////////////////////////////////////////////////
void SoundSource::setPan(float pan)
{
    m_impl->savedSettings.pan = pan;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_pan(sound, pan);
}


////////////////////////////////////////////////////////////
void SoundSource::setVolume(float volume)
{
    m_impl->savedSettings.volume = volume * 0.01f;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_volume(sound, volume * 0.01f);
}


////////////////////////////////////////////////////////////
void SoundSource::setSpatializationEnabled(bool spatializationEnabled)
{
    m_impl->savedSettings.spatializationEnabled = spatializationEnabled;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_spatialization_enabled(sound, spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void SoundSource::setPosition(const Vector3f& position)
{
    m_impl->savedSettings.position = position;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_position(sound, position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setDirection(const Vector3f& direction)
{
    m_impl->savedSettings.direction = direction;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_direction(sound, direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setCone(const Cone& cone)
{
    m_impl->savedSettings.cone = cone;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_cone(sound,
                          priv::clamp(m_impl->savedSettings.cone.innerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                          priv::clamp(m_impl->savedSettings.cone.outerAngle, Angle::Zero, degrees(360.f)).asRadians(),
                          m_impl->savedSettings.cone.outerGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setVelocity(const Vector3f& velocity)
{
    m_impl->savedSettings.velocity = velocity;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_velocity(sound, velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setDopplerFactor(float dopplerFactor)
{
    m_impl->savedSettings.dopplerFactor = dopplerFactor;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_doppler_factor(sound, dopplerFactor);
}


////////////////////////////////////////////////////////////
void SoundSource::setDirectionalAttenuationFactor(float directionalAttenuationFactor)
{
    m_impl->savedSettings.directionalAttenuationFactor = directionalAttenuationFactor;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_directional_attenuation_factor(sound, directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void SoundSource::setRelativeToListener(bool relativeToListener)
{
    m_impl->savedSettings.positioning = relativeToListener ? ma_positioning_relative : ma_positioning_absolute;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_positioning(sound, relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void SoundSource::setMinDistance(float minDistance)
{
    m_impl->savedSettings.minDistance = minDistance;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_min_distance(sound, minDistance);
}


////////////////////////////////////////////////////////////
void SoundSource::setMaxDistance(float maxDistance)
{
    m_impl->savedSettings.maxDistance = maxDistance;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_max_distance(sound, maxDistance);
}


////////////////////////////////////////////////////////////
void SoundSource::setMinGain(float minGain)
{
    m_impl->savedSettings.minGain = minGain;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_min_gain(sound, minGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setMaxGain(float maxGain)
{
    m_impl->savedSettings.maxGain = maxGain;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_max_gain(sound, maxGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setAttenuation(float attenuation)
{
    m_impl->savedSettings.rollOff = attenuation;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_rolloff(sound, attenuation);
}


////////////////////////////////////////////////////////////
void SoundSource::setEffectProcessor(EffectProcessor effectProcessor)
{
    m_impl->effectProcessor = SFML_MOVE(effectProcessor);
}


////////////////////////////////////////////////////////////
void SoundSource::setLoop(bool loop)
{
    m_impl->savedSettings.looping = loop;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void SoundSource::setPlayingOffset(Time playingOffset)
{
    m_impl->playingOffset = playingOffset;
}


////////////////////////////////////////////////////////////
float SoundSource::getPitch() const
{
    return 0.f;
}


////////////////////////////////////////////////////////////
float SoundSource::getPan() const
{
    return m_impl->savedSettings.pan;
}


////////////////////////////////////////////////////////////
float SoundSource::getVolume() const
{
    return m_impl->savedSettings.volume;
}


////////////////////////////////////////////////////////////
bool SoundSource::isSpatializationEnabled() const
{
    return m_impl->savedSettings.spatializationEnabled;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getPosition() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->savedSettings.position;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getDirection() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->savedSettings.direction;
}


////////////////////////////////////////////////////////////
SoundSource::Cone SoundSource::getCone() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->savedSettings.cone;
}


////////////////////////////////////////////////////////////
Vector3f SoundSource::getVelocity() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->savedSettings.velocity;
}


////////////////////////////////////////////////////////////
float SoundSource::getDopplerFactor() const
{
    return m_impl->savedSettings.dopplerFactor;
}


////////////////////////////////////////////////////////////
float SoundSource::getDirectionalAttenuationFactor() const
{
    return m_impl->savedSettings.directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
bool SoundSource::isRelativeToListener() const
{
    return m_impl->savedSettings.positioning == ma_positioning_relative;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinDistance() const
{
    return m_impl->savedSettings.minDistance;
}


////////////////////////////////////////////////////////////
float SoundSource::getMaxDistance() const
{
    return m_impl->savedSettings.maxDistance;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinGain() const
{
    return m_impl->savedSettings.minGain;
}


////////////////////////////////////////////////////////////
float SoundSource::getMaxGain() const
{
    return m_impl->savedSettings.maxGain;
}


////////////////////////////////////////////////////////////
float SoundSource::getAttenuation() const
{
    return m_impl->savedSettings.rollOff;
}


////////////////////////////////////////////////////////////
EffectProcessor SoundSource::getEffectProcessor() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->effectProcessor;
}


////////////////////////////////////////////////////////////
bool SoundSource::getLoop() const
{
    return m_impl->savedSettings.looping;
}


////////////////////////////////////////////////////////////
Time SoundSource::getPlayingOffset() const
{
    return m_impl->playingOffset;
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


////////////////////////////////////////////////////////////
void SoundSource::applyStoredSettings(ma_sound& sound) const
{
    m_impl->savedSettings.applyOnto(sound);
}

} // namespace sf
