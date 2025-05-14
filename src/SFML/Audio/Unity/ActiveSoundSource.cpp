#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ActiveSoundSource.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"
#include "SFML/Audio/SoundBase.hpp"

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
    ma_sound_set_pitch(&getSoundBase().getSound(), pitch);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPan(const float pan)
{
    m_impl->audioSettings.pan = pan;
    ma_sound_set_pan(&getSoundBase().getSound(), pan);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setVolume(const float volume)
{
    SFML_BASE_ASSERT(volume >= 0.f && volume <= 1.f);
    m_impl->audioSettings.volume = volume;
    ma_sound_set_volume(&getSoundBase().getSound(), volume);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setSpatializationEnabled(const bool spatializationEnabled)
{
    m_impl->audioSettings.spatializationEnabled = spatializationEnabled;
    ma_sound_set_spatialization_enabled(&getSoundBase().getSound(), spatializationEnabled ? MA_TRUE : MA_FALSE);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setPosition(const Vec3f& position)
{
    m_impl->audioSettings.position = position;
    ma_sound_set_position(&getSoundBase().getSound(), position.x, position.y, position.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDirection(const Vec3f& direction)
{
    m_impl->audioSettings.direction = direction;
    ma_sound_set_direction(&getSoundBase().getSound(), direction.x, direction.y, direction.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setCone(const Cone& cone)
{
    m_impl->audioSettings.cone = cone;

    ma_sound_set_cone(&getSoundBase().getSound(),
                      base::clamp(cone.innerAngle, Angle::Zero, Angle::Full).asRadians(),
                      base::clamp(cone.outerAngle, Angle::Zero, Angle::Full).asRadians(),
                      cone.outerGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setVelocity(const Vec3f& velocity)
{
    m_impl->audioSettings.velocity = velocity;
    ma_sound_set_velocity(&getSoundBase().getSound(), velocity.x, velocity.y, velocity.z);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDopplerFactor(const float dopplerFactor)
{
    m_impl->audioSettings.dopplerFactor = dopplerFactor;
    ma_sound_set_doppler_factor(&getSoundBase().getSound(), dopplerFactor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setDirectionalAttenuationFactor(const float directionalAttenuationFactor)
{
    m_impl->audioSettings.directionalAttenuationFactor = directionalAttenuationFactor;
    ma_sound_set_directional_attenuation_factor(&getSoundBase().getSound(), directionalAttenuationFactor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setRelativeToListener(const bool relativeToListener)
{
    m_impl->audioSettings.positioning = relativeToListener ? ma_positioning_relative : ma_positioning_absolute;
    ma_sound_set_positioning(&getSoundBase().getSound(),
                             relativeToListener ? ma_positioning_relative : ma_positioning_absolute);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMinDistance(const float minDistance)
{
    m_impl->audioSettings.minDistance = minDistance;
    ma_sound_set_min_distance(&getSoundBase().getSound(), minDistance);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMaxDistance(const float maxDistance)
{
    m_impl->audioSettings.maxDistance = maxDistance;
    ma_sound_set_max_distance(&getSoundBase().getSound(), maxDistance);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMinGain(const float minGain)
{
    m_impl->audioSettings.minGain = minGain;
    ma_sound_set_min_gain(&getSoundBase().getSound(), minGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setMaxGain(const float maxGain)
{
    m_impl->audioSettings.maxGain = maxGain;
    ma_sound_set_max_gain(&getSoundBase().getSound(), maxGain);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setAttenuation(const float attenuation)
{
    m_impl->audioSettings.rollOff = attenuation;
    ma_sound_set_rolloff(&getSoundBase().getSound(), attenuation);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    m_impl->effectProcessor = effectProcessor;
    getSoundBase().setAndConnectEffectProcessor(effectProcessor);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::setLooping(const bool loop)
{
    m_impl->audioSettings.looping = loop;
    ma_sound_set_looping(&getSoundBase().getSound(), loop ? MA_TRUE : MA_FALSE);
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
    return m_impl->audioSettings.positioning == 1; // ma_positioning_relative TODO P0: use enum class
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
void ActiveSoundSource::applyAudioSettings(const AudioSettings& settings)
{
    getSoundBase().applyAudioSettings(settings);
}


////////////////////////////////////////////////////////////
void ActiveSoundSource::applySettingsAndEffectProcessorTo(priv::MiniaudioUtils::SoundBase& soundBase) const
{
    soundBase.applyAudioSettings(m_impl->audioSettings);
    soundBase.setAndConnectEffectProcessor(m_impl->effectProcessor);
}


////////////////////////////////////////////////////////////
Time ActiveSoundSource::getPlayingOffset() const
{
    return priv::MiniaudioUtils::getPlayingOffset(getSoundBase().getSound()).value();
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::resume()
{
    if (m_playing)
        return true;

    if (const ma_result result = ma_sound_start(&getSoundBase().sound); result != MA_SUCCESS)
        return priv::MiniaudioUtils::fail("start playing audio stream", result);

    m_playing = true;
    return true;
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::pause()
{
    if (!m_playing)
        return true;

    if (const ma_result result = ma_sound_stop(&getSoundBase().sound); result != MA_SUCCESS)
        return priv::MiniaudioUtils::fail("stop playing audio stream", result);

    m_playing = false;
    return true;
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::isPlaying() const
{
    return m_playing;
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::play(const Time playingOffset)
{
    setPlayingOffset(playingOffset);
    return resume();
}


////////////////////////////////////////////////////////////
bool ActiveSoundSource::stop()
{
    setPlayingOffset(Time{});
    return pause();
}

} // namespace sf
