#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/Priv/MiniaudioSoundSource.hpp"
#include "SFML/Audio/SoundBase.hpp"

#include "SFML/Base/Clamp.hpp"

#include <miniaudio.h>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct MiniaudioSoundSource::Impl
{
    AudioSettings   audioSettings;
    EffectProcessor effectProcessor{};
};


////////////////////////////////////////////////////////////
MiniaudioSoundSource::MiniaudioSoundSource()                                           = default;
MiniaudioSoundSource::MiniaudioSoundSource(const MiniaudioSoundSource&)                = default;
MiniaudioSoundSource::MiniaudioSoundSource(MiniaudioSoundSource&&) noexcept            = default;
MiniaudioSoundSource& MiniaudioSoundSource::operator=(const MiniaudioSoundSource& rhs) = default;
MiniaudioSoundSource& MiniaudioSoundSource::operator=(MiniaudioSoundSource&&) noexcept = default;
MiniaudioSoundSource::~MiniaudioSoundSource()                                          = default;


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setPitch(const float pitch)
{
    m_impl->audioSettings.pitch = pitch;
    ma_sound_set_pitch(&getSoundBase().getSound(), pitch);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setPan(const float pan)
{
    m_impl->audioSettings.pan = pan;
    ma_sound_set_pan(&getSoundBase().getSound(), pan);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setVolume(const float volume)
{
    SFML_BASE_ASSERT(volume >= 0.f && volume <= 1.f);
    m_impl->audioSettings.volume = volume;
    ma_sound_set_volume(&getSoundBase().getSound(), volume);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setSpatializationEnabled(const bool spatializationEnabled)
{
    m_impl->audioSettings.spatializationEnabled = spatializationEnabled;
    ma_sound_set_spatialization_enabled(&getSoundBase().getSound(), spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setPosition(const Vec3f& position)
{
    m_impl->audioSettings.position = position;
    ma_sound_set_position(&getSoundBase().getSound(), position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setDirection(const Vec3f& direction)
{
    m_impl->audioSettings.direction = direction;
    ma_sound_set_direction(&getSoundBase().getSound(), direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setCone(const Cone& cone)
{
    m_impl->audioSettings.cone = cone;

    ma_sound_set_cone(&getSoundBase().getSound(),
                      base::clamp(cone.innerAngle, Angle::Zero, Angle::Full).asRadians(),
                      base::clamp(cone.outerAngle, Angle::Zero, Angle::Full).asRadians(),
                      cone.outerGain);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setVelocity(const Vec3f& velocity)
{
    m_impl->audioSettings.velocity = velocity;
    ma_sound_set_velocity(&getSoundBase().getSound(), velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setDopplerFactor(const float dopplerFactor)
{
    m_impl->audioSettings.dopplerFactor = dopplerFactor;
    ma_sound_set_doppler_factor(&getSoundBase().getSound(), dopplerFactor);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setDirectionalAttenuationFactor(const float directionalAttenuationFactor)
{
    m_impl->audioSettings.directionalAttenuationFactor = directionalAttenuationFactor;
    ma_sound_set_directional_attenuation_factor(&getSoundBase().getSound(), directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setRelativeToListener(const bool relativeToListener)
{
    m_impl->audioSettings.positioning = relativeToListener ? AudioSettings::Positioning::Relative
                                                           : AudioSettings::Positioning::Absolute;

    ma_sound_set_positioning(&getSoundBase().getSound(),
                             relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setMinDistance(const float minDistance)
{
    m_impl->audioSettings.minDistance = minDistance;
    ma_sound_set_min_distance(&getSoundBase().getSound(), minDistance);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setMaxDistance(const float maxDistance)
{
    m_impl->audioSettings.maxDistance = maxDistance;
    ma_sound_set_max_distance(&getSoundBase().getSound(), maxDistance);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setMinGain(const float minGain)
{
    m_impl->audioSettings.minGain = minGain;
    ma_sound_set_min_gain(&getSoundBase().getSound(), minGain);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setMaxGain(const float maxGain)
{
    m_impl->audioSettings.maxGain = maxGain;
    ma_sound_set_max_gain(&getSoundBase().getSound(), maxGain);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setAttenuation(const float attenuation)
{
    m_impl->audioSettings.attenuation = attenuation;
    ma_sound_set_rolloff(&getSoundBase().getSound(), attenuation);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    m_impl->effectProcessor = effectProcessor;
    getSoundBase().setAndConnectEffectProcessor(effectProcessor);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::setLooping(const bool loop)
{
    m_impl->audioSettings.looping = loop;
    ma_sound_set_looping(&getSoundBase().getSound(), loop ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getPitch() const
{
    return m_impl->audioSettings.pitch;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getPan() const
{
    return m_impl->audioSettings.pan;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getVolume() const
{
    const float result = m_impl->audioSettings.volume;
    SFML_BASE_ASSERT(result >= 0.f && result <= 1.f);
    return result;
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::isSpatializationEnabled() const
{
    return m_impl->audioSettings.spatializationEnabled;
}


////////////////////////////////////////////////////////////
Vec3f MiniaudioSoundSource::getPosition() const
{
    return m_impl->audioSettings.position;
}


////////////////////////////////////////////////////////////
Vec3f MiniaudioSoundSource::getDirection() const
{
    return m_impl->audioSettings.direction;
}


////////////////////////////////////////////////////////////
MiniaudioSoundSource::Cone MiniaudioSoundSource::getCone() const
{
    return m_impl->audioSettings.cone;
}


////////////////////////////////////////////////////////////
Vec3f MiniaudioSoundSource::getVelocity() const
{
    return m_impl->audioSettings.velocity;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getDopplerFactor() const
{
    return m_impl->audioSettings.dopplerFactor;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getDirectionalAttenuationFactor() const
{
    return m_impl->audioSettings.directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::isRelativeToListener() const
{
    return m_impl->audioSettings.positioning == AudioSettings::Positioning::Relative;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getMinDistance() const
{
    return m_impl->audioSettings.minDistance;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getMaxDistance() const
{
    return m_impl->audioSettings.maxDistance;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getMinGain() const
{
    return m_impl->audioSettings.minGain;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getMaxGain() const
{
    return m_impl->audioSettings.maxGain;
}


////////////////////////////////////////////////////////////
float MiniaudioSoundSource::getAttenuation() const
{
    return m_impl->audioSettings.attenuation;
}


////////////////////////////////////////////////////////////
const EffectProcessor& MiniaudioSoundSource::getEffectProcessor() const
{
    return m_impl->effectProcessor;
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::isLooping() const
{
    return m_impl->audioSettings.looping;
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::applyAudioSettings(const AudioSettings& settings)
{
    getSoundBase().applyAudioSettings(settings);
}


////////////////////////////////////////////////////////////
void MiniaudioSoundSource::applySettingsAndEffectProcessorTo(MiniaudioUtils::SoundBase& soundBase) const
{
    soundBase.applyAudioSettings(m_impl->audioSettings);
    soundBase.setAndConnectEffectProcessor(m_impl->effectProcessor);
}


////////////////////////////////////////////////////////////
Time MiniaudioSoundSource::getPlayingOffset() const
{
    return MiniaudioUtils::getPlayingOffset(getSoundBase().getSound()).value();
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::resume()
{
    if (m_playing)
        return true;

    if (const ma_result result = ma_sound_start(&getSoundBase().sound); result != MA_SUCCESS)
        return MiniaudioUtils::fail("start playing audio source", result);

    m_playing = true;
    return true;
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::pause()
{
    if (!m_playing)
        return true;

    if (const ma_result result = ma_sound_stop(&getSoundBase().sound); result != MA_SUCCESS)
        return MiniaudioUtils::fail("stop playing audio source", result);

    m_playing = false;
    return true;
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::isPlaying() const
{
    return m_playing;
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::play(const Time playingOffset)
{
    setPlayingOffset(playingOffset);
    return resume();
}


////////////////////////////////////////////////////////////
bool MiniaudioSoundSource::stop()
{
    setPlayingOffset(Time{});
    return pause();
}

} // namespace sf::priv
