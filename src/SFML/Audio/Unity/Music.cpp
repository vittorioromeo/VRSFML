// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Music.hpp"

#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/InputSoundFile.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/SoundStream.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/LifetimeDependant.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Vector.hpp"

#include <mutex>


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
struct Music::Impl
{
    base::Vector<base::I16> samples;        //!< Temporary buffer of samples
    mutable std::mutex      loopMutex;      //!< Protects loopSpan and sampleOffset
    Span<base::U64>         loopSpan;       //!< Loop range Specifier
    MusicReader&            musicReader;    //!< The music reader
    base::U64               sampleOffset{}; //!< Current offset in the stream

    explicit Impl(MusicReader& theMusicReader, base::U64 theSampleCount) :
        loopSpan{0u, theSampleCount},
        musicReader(theMusicReader)
    {
    }

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(MusicReader);
};


////////////////////////////////////////////////////////////
Music::Music(PlaybackDevice& playbackDevice, MusicReader& musicReader, const AudioSettings& audioSettings) :
    SoundStream(playbackDevice, musicReader.getChannelMap(), musicReader.getSampleRate()),
    m_impl(musicReader, musicReader.getSampleCount())
{
    SFML_UPDATE_LIFETIME_DEPENDANT(MusicReader, Music, (&*m_impl), (&m_impl->musicReader));
    applyAudioSettings(audioSettings);
}


////////////////////////////////////////////////////////////
Music::Music(PlaybackDevice& playbackDevice, MusicReader& musicReader) :
    Music{playbackDevice, musicReader, AudioSettings{}}
{
}


////////////////////////////////////////////////////////////
Music::~Music()
{
#ifdef SFML_ENABLE_LIFETIME_TRACKING
    if (m_impl->m_sfPrivLifetimeDependantMusicReader.isTestingModeErrorTriggered())
        return;
#endif

    // TODO P1: revisit?
    // Stop the sound before `m_impl->samples` is destroyed, otherwise the audio
    // callback can race with `onGetData()` during derived-member teardown.
    pause();
}


////////////////////////////////////////////////////////////
bool Music::onGetData(SoundStream::Chunk& data)
{
    const std::lock_guard lock(m_impl->loopMutex);

    // Resize the internal buffer so that it can contain 1 second of audio samples
    m_impl->samples.resize(m_impl->musicReader.getSampleRate() * m_impl->musicReader.getChannelCount());

    base::SizeT     toFill  = m_impl->samples.size();
    const base::U64 loopEnd = m_impl->loopSpan.offset + m_impl->loopSpan.length;

    // If the loop end is enabled and imminent, request less data.
    // This will trip an "onLoop()" call from the underlying SoundStream,
    // and we can then take action.
    if (isLooping() && (m_impl->loopSpan.length != 0) && (m_impl->sampleOffset <= loopEnd) &&
        (m_impl->sampleOffset + toFill > loopEnd))
        toFill = static_cast<base::SizeT>(loopEnd - m_impl->sampleOffset);

    // Fill the chunk parameters
    data.samples = m_impl->samples.data();

    // `seekAndRead` is thread-safe
    const auto [sampleOffset,
                samplesRead] = m_impl->musicReader.seekAndRead(m_impl->sampleOffset, m_impl->samples.data(), toFill);

    data.sampleCount     = static_cast<base::SizeT>(samplesRead);
    m_impl->sampleOffset = sampleOffset + samplesRead;

    // Check if we have stopped obtaining samples or reached either the EOF or the loop end point
    return (data.sampleCount != 0) && (m_impl->sampleOffset < m_impl->musicReader.getSampleCount()) &&
           (m_impl->sampleOffset != loopEnd || m_impl->loopSpan.length == 0);
}


////////////////////////////////////////////////////////////
void Music::onSeek(const Time timeOffset)
{
    const std::lock_guard lock(m_impl->loopMutex);
    m_impl->sampleOffset = timeToSamples(m_impl->musicReader.getSampleRate(), m_impl->musicReader.getChannelCount(), timeOffset);
}


////////////////////////////////////////////////////////////
// Called by underlying SoundStream so we can determine where to loop.
base::Optional<base::U64> Music::onLoop()
{
    const std::lock_guard lock(m_impl->loopMutex);

    if (isLooping() && (m_impl->loopSpan.length != 0) &&
        (m_impl->sampleOffset == m_impl->loopSpan.offset + m_impl->loopSpan.length))
    {
        // Looping is enabled, and either we're at the loop end, or we're at the EOF
        // when it's equivalent to the loop end (loop end takes priority). Send us to loop begin
        m_impl->sampleOffset = m_impl->loopSpan.offset;
        return base::makeOptional(m_impl->sampleOffset);
    }

    if (isLooping() && (m_impl->sampleOffset >= m_impl->musicReader.getSampleCount()))
    {
        // If we're at the EOF, reset to 0
        m_impl->sampleOffset = 0u;
        return base::makeOptional(m_impl->sampleOffset);
    }

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
Music::TimeSpan Music::getLoopPoints() const
{
    const std::lock_guard lock(m_impl->loopMutex);

    const auto sampleRate   = m_impl->musicReader.getSampleRate();
    const auto channelCount = m_impl->musicReader.getChannelCount();

    return TimeSpan{samplesToTime(sampleRate, channelCount, m_impl->loopSpan.offset),
                    samplesToTime(sampleRate, channelCount, m_impl->loopSpan.length)};
}


////////////////////////////////////////////////////////////
void Music::setLoopPoints(const TimeSpan timePoints)
{
    const auto sampleRate   = m_impl->musicReader.getSampleRate();
    const auto channelCount = m_impl->musicReader.getChannelCount();

    SFML_BASE_ASSERT(channelCount > 0u);
    SFML_BASE_ASSERT(sampleRate > 0u);

    const auto fileSampleCount = m_impl->musicReader.getSampleCount();

    Span<base::U64> samplePoints{timeToSamples(sampleRate, channelCount, timePoints.offset),
                                 timeToSamples(sampleRate, channelCount, timePoints.length)};

    // Check our state. This averts a divide-by-zero.
    if (fileSampleCount == 0u)
    {
        priv::err() << "Music is not in a valid state to assign Loop Points.";
        return;
    }

    // Round up to the next even sample if needed
    samplePoints.offset += (channelCount - 1u);
    samplePoints.offset -= (samplePoints.offset % channelCount);
    samplePoints.length += (channelCount - 1u);
    samplePoints.length -= (samplePoints.length % channelCount);

    // Validate
    if (samplePoints.offset >= fileSampleCount)
    {
        priv::err() << "LoopPoints offset val must be in range [0, Duration).";
        m_impl->loopSpan = {0u, m_impl->musicReader.getSampleCount()}; // Reset to default
        return;
    }

    if (samplePoints.length == 0u)
    {
        priv::err() << "LoopPoints length val must be nonzero.";
        m_impl->loopSpan = {0u, m_impl->musicReader.getSampleCount()}; // Reset to default
        return;
    }

    // Clamp End Point
    samplePoints.length = base::min(samplePoints.length, fileSampleCount - samplePoints.offset);

    // If this change has no effect, we can return without touching anything
    if (samplePoints.offset == m_impl->loopSpan.offset && samplePoints.length == m_impl->loopSpan.length)
        return;

    // When we apply this change, we need to "reset" this instance and its buffer

    // Get old playing status and position
    const bool oldIsPlaying = isPlaying();
    const Time oldPos       = getPlayingOffset();

    // Unload
    stop();

    // Set
    m_impl->loopSpan = samplePoints;

    // Restore
    if (oldPos != Time{})
        setPlayingOffset(oldPos);

    // Resume
    if (oldIsPlaying)
        resume();
}


////////////////////////////////////////////////////////////
const MusicReader& Music::getMusicReader() const
{
    return m_impl->musicReader;
}

} // namespace sf
