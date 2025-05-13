#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/EffectProcessor.hpp"
#include "SFML/Audio/InactiveSoundSource.hpp"
#include "SFML/Audio/MiniaudioUtils.hpp"

#include <miniaudio.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct InactiveSoundSource::Impl
{
    AudioSettings   audioSettings;
    EffectProcessor effectProcessor{};
};


////////////////////////////////////////////////////////////
InactiveSoundSource::InactiveSoundSource() = default;


////////////////////////////////////////////////////////////
InactiveSoundSource::InactiveSoundSource(const InactiveSoundSource&) = default;


////////////////////////////////////////////////////////////
InactiveSoundSource::InactiveSoundSource(InactiveSoundSource&&) noexcept = default;


////////////////////////////////////////////////////////////
InactiveSoundSource& InactiveSoundSource::operator=(const InactiveSoundSource& rhs) = default;


////////////////////////////////////////////////////////////
InactiveSoundSource& InactiveSoundSource::operator=(InactiveSoundSource&&) noexcept = default;


////////////////////////////////////////////////////////////
InactiveSoundSource::~InactiveSoundSource() = default;


////////////////////////////////////////////////////////////
void InactiveSoundSource::setPitch(const float pitch)
{
    m_impl->audioSettings.pitch = pitch;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setPan(const float pan)
{
    m_impl->audioSettings.pan = pan;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setVolume(const float volume)
{
    SFML_BASE_ASSERT(volume >= 0.f && volume <= 1.f);
    m_impl->audioSettings.volume = volume;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setSpatializationEnabled(const bool spatializationEnabled)
{
    m_impl->audioSettings.spatializationEnabled = spatializationEnabled;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setPosition(const Vec3f& position)
{
    m_impl->audioSettings.position = position;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setDirection(const Vec3f& direction)
{
    m_impl->audioSettings.direction = direction;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setCone(const Cone& cone)
{
    m_impl->audioSettings.cone = cone;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setVelocity(const Vec3f& velocity)
{
    m_impl->audioSettings.velocity = velocity;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setDopplerFactor(const float dopplerFactor)
{
    m_impl->audioSettings.dopplerFactor = dopplerFactor;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setDirectionalAttenuationFactor(const float directionalAttenuationFactor)
{
    m_impl->audioSettings.directionalAttenuationFactor = directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setRelativeToListener(const bool relativeToListener)
{
    m_impl->audioSettings.positioning = relativeToListener ? ma_positioning_relative : ma_positioning_absolute;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setMinDistance(const float minDistance)
{
    m_impl->audioSettings.minDistance = minDistance;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setMaxDistance(const float maxDistance)
{
    m_impl->audioSettings.maxDistance = maxDistance;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setMinGain(const float minGain)
{
    m_impl->audioSettings.minGain = minGain;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setMaxGain(const float maxGain)
{
    m_impl->audioSettings.maxGain = maxGain;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setAttenuation(const float attenuation)
{
    m_impl->audioSettings.rollOff = attenuation;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setEffectProcessor(const EffectProcessor& effectProcessor)
{
    m_impl->effectProcessor = effectProcessor;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::setLooping(const bool loop)
{
    m_impl->audioSettings.looping = loop;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getPitch() const
{
    return m_impl->audioSettings.pitch;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getPan() const
{
    return m_impl->audioSettings.pan;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getVolume() const
{
    const float result = m_impl->audioSettings.volume;
    SFML_BASE_ASSERT(result >= 0.f && result <= 1.f);
    return result;
}


////////////////////////////////////////////////////////////
bool InactiveSoundSource::isSpatializationEnabled() const
{
    return m_impl->audioSettings.spatializationEnabled;
}


////////////////////////////////////////////////////////////
Vec3f InactiveSoundSource::getPosition() const
{
    return m_impl->audioSettings.position;
}


////////////////////////////////////////////////////////////
Vec3f InactiveSoundSource::getDirection() const
{
    return m_impl->audioSettings.direction;
}


////////////////////////////////////////////////////////////
InactiveSoundSource::Cone InactiveSoundSource::getCone() const
{
    return m_impl->audioSettings.cone;
}


////////////////////////////////////////////////////////////
Vec3f InactiveSoundSource::getVelocity() const
{
    return m_impl->audioSettings.velocity;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getDopplerFactor() const
{
    return m_impl->audioSettings.dopplerFactor;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getDirectionalAttenuationFactor() const
{
    return m_impl->audioSettings.directionalAttenuationFactor;
}


////////////////////////////////////////////////////////////
bool InactiveSoundSource::isRelativeToListener() const
{
    return m_impl->audioSettings.positioning == 1; // ma_positioning_relative TODO P0: use enum class
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getMinDistance() const
{
    return m_impl->audioSettings.minDistance;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getMaxDistance() const
{
    return m_impl->audioSettings.maxDistance;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getMinGain() const
{
    return m_impl->audioSettings.minGain;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getMaxGain() const
{
    return m_impl->audioSettings.maxGain;
}


////////////////////////////////////////////////////////////
float InactiveSoundSource::getAttenuation() const
{
    return m_impl->audioSettings.rollOff;
}


////////////////////////////////////////////////////////////
const EffectProcessor& InactiveSoundSource::getEffectProcessor() const
{
    return m_impl->effectProcessor;
}


////////////////////////////////////////////////////////////
bool InactiveSoundSource::isLooping() const
{
    return m_impl->audioSettings.looping;
}


////////////////////////////////////////////////////////////
void InactiveSoundSource::applySettingsAndEffectProcessorTo(priv::MiniaudioUtils::SoundBase& soundBase) const
{
    soundBase.applyAudioSettings(m_impl->audioSettings);
    soundBase.setAndConnectEffectProcessor(m_impl->effectProcessor);
}

} // namespace sf
