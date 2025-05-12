#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/SoundSource.hpp"

#include "SFML/System/Time.hpp"

#include "SFML/Base/Clamp.hpp"
#include "SFML/Base/Macros.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct SoundSource::Impl
{
    AudioSettings   audioSettings;
    EffectProcessor effectProcessor{};
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
void SoundSource::setPitch(const float pitch)
{
    m_impl->audioSettings.pitch = pitch;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_pitch(sound, pitch);
}


////////////////////////////////////////////////////////////
void SoundSource::setPan(const float pan)
{
    m_impl->audioSettings.pan = pan;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_pan(sound, pan);
}


////////////////////////////////////////////////////////////
void SoundSource::setVolume(const float volume)
{
    SFML_BASE_ASSERT(volume >= 0.f && volume <= 1.f);

    m_impl->audioSettings.volume = volume;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_volume(sound, volume);
}


////////////////////////////////////////////////////////////
void SoundSource::setSpatializationEnabled(bool spatializationEnabled)
{
    m_impl->audioSettings.spatializationEnabled = spatializationEnabled;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_spatialization_enabled(sound, spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void SoundSource::setPosition(const Vec3f& position)
{
    m_impl->audioSettings.position = position;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_position(sound, position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setDirection(const Vec3f& direction)
{
    m_impl->audioSettings.direction = direction;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_direction(sound, direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setCone(const Cone& cone)
{
    m_impl->audioSettings.cone = cone;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_cone(sound,
                          base::clamp(m_impl->audioSettings.cone.innerAngle, Angle::Zero, Angle::Full).asRadians(),
                          base::clamp(m_impl->audioSettings.cone.outerAngle, Angle::Zero, Angle::Full).asRadians(),
                          m_impl->audioSettings.cone.outerGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setVelocity(const Vec3f& velocity)
{
    m_impl->audioSettings.velocity = velocity;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_velocity(sound, velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
void SoundSource::setDopplerFactor(const float dopplerFactor)
{
    m_impl->audioSettings.dopplerFactor = dopplerFactor;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_doppler_factor(sound, dopplerFactor);
}


////////////////////////////////////////////////////////////
void SoundSource::setDirectionalAttenuationFactor(const float directionalAttenuationFactor)
{
    m_impl->audioSettings.directionalAttenuationFactor = directionalAttenuationFactor;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_directional_attenuation_factor(sound, directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void SoundSource::setRelativeToListener(bool relativeToListener)
{
    m_impl->audioSettings.positioning = relativeToListener ? ma_positioning_relative : ma_positioning_absolute;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_positioning(sound, relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void SoundSource::setMinDistance(const float minDistance)
{
    m_impl->audioSettings.minDistance = minDistance;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_min_distance(sound, minDistance);
}


////////////////////////////////////////////////////////////
void SoundSource::setMaxDistance(const float maxDistance)
{
    m_impl->audioSettings.maxDistance = maxDistance;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_max_distance(sound, maxDistance);
}


////////////////////////////////////////////////////////////
void SoundSource::setMinGain(const float minGain)
{
    m_impl->audioSettings.minGain = minGain;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_min_gain(sound, minGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setMaxGain(const float maxGain)
{
    m_impl->audioSettings.maxGain = maxGain;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_max_gain(sound, maxGain);
}


////////////////////////////////////////////////////////////
void SoundSource::setAttenuation(const float attenuation)
{
    m_impl->audioSettings.rollOff = attenuation;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_rolloff(sound, attenuation);
}


////////////////////////////////////////////////////////////
void SoundSource::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    m_impl->effectProcessor = effectProcessor;
}


////////////////////////////////////////////////////////////
void SoundSource::setLooping(bool loop)
{
    m_impl->audioSettings.looping = loop;

    if (auto* sound = static_cast<ma_sound*>(getSound()))
        ma_sound_set_looping(sound, loop ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
float SoundSource::getPitch() const
{
    return m_impl->audioSettings.pitch;
}


////////////////////////////////////////////////////////////
float SoundSource::getPan() const
{
    return m_impl->audioSettings.pan;
}


////////////////////////////////////////////////////////////
float SoundSource::getVolume() const
{
    const float result = m_impl->audioSettings.volume;
    SFML_BASE_ASSERT(result >= 0.f && result <= 1.f);
    return result;
}


////////////////////////////////////////////////////////////
bool SoundSource::isSpatializationEnabled() const
{
    return m_impl->audioSettings.spatializationEnabled;
}


////////////////////////////////////////////////////////////
Vec3f SoundSource::getPosition() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->audioSettings.position;
}


////////////////////////////////////////////////////////////
Vec3f SoundSource::getDirection() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->audioSettings.direction;
}


////////////////////////////////////////////////////////////
SoundSource::Cone SoundSource::getCone() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->audioSettings.cone;
}


////////////////////////////////////////////////////////////
Vec3f SoundSource::getVelocity() const
{
    // NOLINTNEXTLINE(modernize-return-braced-init-list)
    return m_impl->audioSettings.velocity;
}


////////////////////////////////////////////////////////////
float SoundSource::getDopplerFactor() const
{
    return m_impl->audioSettings.dopplerFactor;
}


////////////////////////////////////////////////////////////
float SoundSource::getDirectionalAttenuationFactor() const
{
    return m_impl->audioSettings.directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
bool SoundSource::isRelativeToListener() const
{
    return m_impl->audioSettings.positioning == ma_positioning_relative;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinDistance() const
{
    return m_impl->audioSettings.minDistance;
}


////////////////////////////////////////////////////////////
float SoundSource::getMaxDistance() const
{
    return m_impl->audioSettings.maxDistance;
}


////////////////////////////////////////////////////////////
float SoundSource::getMinGain() const
{
    return m_impl->audioSettings.minGain;
}


////////////////////////////////////////////////////////////
float SoundSource::getMaxGain() const
{
    return m_impl->audioSettings.maxGain;
}


////////////////////////////////////////////////////////////
float SoundSource::getAttenuation() const
{
    return m_impl->audioSettings.rollOff;
}


////////////////////////////////////////////////////////////
const EffectProcessor& SoundSource::getEffectProcessor() const
{
    return m_impl->effectProcessor;
}


////////////////////////////////////////////////////////////
bool SoundSource::isLooping() const
{
    return m_impl->audioSettings.looping;
}


////////////////////////////////////////////////////////////
SoundSource& SoundSource::operator=(const SoundSource& rhs)
{
    if (this == &rhs)
        return *this;

    setPitch(rhs.getPitch());
    setPan(rhs.getPan());
    setVolume(rhs.getVolume());
    setSpatializationEnabled(rhs.isSpatializationEnabled());
    setPosition(rhs.getPosition());
    setDirection(rhs.getDirection());
    setCone(rhs.getCone());
    setVelocity(rhs.getVelocity());
    setDopplerFactor(rhs.getDopplerFactor());
    setRelativeToListener(rhs.isRelativeToListener());
    setMinDistance(rhs.getMinDistance());
    setMaxDistance(rhs.getMaxDistance());
    setMinGain(rhs.getMinGain());
    setMaxGain(rhs.getMaxGain());
    setAttenuation(rhs.getAttenuation());
    setEffectProcessor(rhs.getEffectProcessor());
    setLooping(rhs.isLooping());
    setPlayingOffset(rhs.getPlayingOffset());

    return *this;
}


////////////////////////////////////////////////////////////
void SoundSource::applyAudioSettings(ma_sound& sound) const
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
