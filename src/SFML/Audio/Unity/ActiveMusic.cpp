#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ActiveMusic.hpp"
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/InputSoundFile.hpp"
#include "SFML/Audio/MusicSource.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Vector.hpp"


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr sf::Time samplesToTime(
    const unsigned int  sampleRate,
    const unsigned int  channelCount,
    const sf::base::U64 samples)
{
    SFML_BASE_ASSERT(sampleRate > 0u);
    SFML_BASE_ASSERT(channelCount > 0u);

    return sf::microseconds(static_cast<sf::base::I64>((samples * 1'000'000u) / (channelCount * sampleRate)));
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr sf::base::U64 timeToSamples(
    const unsigned int sampleRate,
    const unsigned int channelCount,
    const sf::Time     position)
{
    // Always ROUND, no unchecked truncation, hence the addition in the numerator.
    // This avoids most precision errors arising from "samples => Time => samples" conversions
    // Original rounding calculation is ((Micros * Freq * Channels) / 1'000'000) + 0.5
    // We refactor it to keep sf::base::I64 as the data type throughout the whole operation.
    return ((static_cast<sf::base::U64>(position.asMicroseconds()) * sampleRate * channelCount) + 500'000u) / 1'000'000u;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
ActiveMusic::ActiveMusic(PlaybackDevice& playbackDevice, MusicSource& musicSource, const AudioSettings& audioSettings) :
ActiveSoundStream(playbackDevice, musicSource.getChannelMap(), musicSource.getSampleRate()),
m_loopSpan{0u, musicSource.getSampleCount()},
m_musicSource(&musicSource),
m_sampleOffset{0u}
{
    SFML_UPDATE_LIFETIME_DEPENDANT(MusicSource, ActiveMusic, this, m_musicSource);

    // TODO P0: needed???
    applySettings(audioSettings);
    setEffectProcessor(getEffectProcessor());
    setPlayingOffset(getPlayingOffset());
}


////////////////////////////////////////////////////////////
ActiveMusic::~ActiveMusic() = default;


////////////////////////////////////////////////////////////
bool ActiveMusic::onGetData(ActiveSoundStream::Chunk& data)
{
    // Resize the internal buffer so that it can contain 1 second of audio samples
    m_samples.resize(getSampleRate() * getChannelCount());

    base::SizeT     toFill  = m_samples.size();
    const base::U64 loopEnd = m_loopSpan.offset + m_loopSpan.length;

    // If the loop end is enabled and imminent, request less data.
    // This will trip an "onLoop()" call from the underlying ActiveSoundStream,
    // and we can then take action.
    if (isLooping() && (m_loopSpan.length != 0) && (m_sampleOffset <= loopEnd) && (m_sampleOffset + toFill > loopEnd))
        toFill = static_cast<base::SizeT>(loopEnd - m_sampleOffset);

    // Fill the chunk parameters
    data.samples = m_samples.data();

    // `seekAndRead` is thread-safe
    const auto [sampleOffset, samplesRead] = m_musicSource->seekAndRead(m_sampleOffset, m_samples.data(), toFill);

    data.sampleCount = samplesRead;
    m_sampleOffset   = sampleOffset + samplesRead;

    // Check if we have stopped obtaining samples or reached either the EOF or the loop end point
    return (data.sampleCount != 0) && (m_sampleOffset < m_musicSource->getSampleCount()) &&
           (m_sampleOffset != loopEnd || m_loopSpan.length == 0);
}


////////////////////////////////////////////////////////////
void ActiveMusic::onSeek(const Time timeOffset)
{
    m_sampleOffset = timeToSamples(getSampleRate(), getChannelCount(), timeOffset);
}


////////////////////////////////////////////////////////////
// Called by underlying ActiveSoundStream so we can determine where to loop.
base::Optional<base::U64> ActiveMusic::onLoop()
{
    if (isLooping() && (m_loopSpan.length != 0) && (m_sampleOffset == m_loopSpan.offset + m_loopSpan.length))
    {
        // Looping is enabled, and either we're at the loop end, or we're at the EOF
        // when it's equivalent to the loop end (loop end takes priority). Send us to loop begin
        m_sampleOffset = m_loopSpan.offset;
        return base::makeOptional(m_sampleOffset);
    }

    if (isLooping() && (m_sampleOffset >= m_musicSource->getSampleCount()))
    {
        // If we're at the EOF, reset to 0
        m_sampleOffset = 0u;
        return base::makeOptional(m_sampleOffset);
    }

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
ActiveMusic::TimeSpan ActiveMusic::getLoopPoints() const
{
    const auto sampleRate   = getSampleRate();
    const auto channelCount = getChannelCount();

    return TimeSpan{samplesToTime(sampleRate, channelCount, m_loopSpan.offset),
                    samplesToTime(sampleRate, channelCount, m_loopSpan.length)};
}


////////////////////////////////////////////////////////////
void ActiveMusic::setLoopPoints(const TimeSpan timePoints)
{
    const auto sampleRate   = getSampleRate();
    const auto channelCount = getChannelCount();

    SFML_BASE_ASSERT(channelCount > 0u);
    SFML_BASE_ASSERT(sampleRate > 0u);

    const auto fileChannelCount = m_musicSource->getSampleCount();

    Span<base::U64> samplePoints{timeToSamples(sampleRate, channelCount, timePoints.offset),
                                 timeToSamples(sampleRate, channelCount, timePoints.length)};

    // Check our state. This averts a divide-by-zero.
    if (fileChannelCount == 0u)
    {
        priv::err() << "ActiveMusic is not in a valid state to assign Loop Points.";
        return;
    }

    // Round up to the next even sample if needed
    samplePoints.offset += (channelCount - 1u);
    samplePoints.offset -= (samplePoints.offset % channelCount);
    samplePoints.length += (channelCount - 1u);
    samplePoints.length -= (samplePoints.length % channelCount);

    // Validate
    if (samplePoints.offset >= fileChannelCount)
    {
        priv::err() << "LoopPoints offset val must be in range [0, Duration).";
        m_loopSpan = {0u, m_musicSource->getSampleCount()}; // Reset to default
        return;
    }

    if (samplePoints.length == 0u)
    {
        priv::err() << "LoopPoints length val must be nonzero.";
        m_loopSpan = {0u, m_musicSource->getSampleCount()}; // Reset to default
        return;
    }

    // Clamp End Point
    samplePoints.length = base::min(samplePoints.length, fileChannelCount - samplePoints.offset);

    // If this change has no effect, we can return without touching anything
    if (samplePoints.offset == m_loopSpan.offset && samplePoints.length == m_loopSpan.length)
        return;

    // When we apply this change, we need to "reset" this instance and its buffer

    // Get old playing status and position
    // const Status oldStatus = getStatus();
    const Time oldPos = getPlayingOffset();

    // Unload
    // stop();

    // Set
    m_loopSpan = samplePoints;

    // Restore
    if (oldPos != Time{})
        setPlayingOffset(oldPos);

    // Resume
    // if (oldStatus == Status::Playing)
    //    resumeOnLastPlaybackDevice();
}


////////////////////////////////////////////////////////////
unsigned int ActiveMusic::getChannelCount() const
{
    return m_musicSource->getChannelCount();
}


////////////////////////////////////////////////////////////
unsigned int ActiveMusic::getSampleRate() const
{
    return m_musicSource->getSampleRate();
}


////////////////////////////////////////////////////////////
sf::ChannelMap ActiveMusic::getChannelMap() const
{
    return m_musicSource->getChannelMap();
}

} // namespace sf
