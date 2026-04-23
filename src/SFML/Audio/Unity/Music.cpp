// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Music.hpp"

#include "SFML/Audio/AudioSettings.hpp"
#include "SFML/Audio/MusicReader.hpp"
#include "SFML/Audio/Priv/MiniaudioSoundSource.hpp"
#include "SFML/Audio/Priv/MiniaudioUtils.hpp"
#include "SFML/Audio/Priv/SoundBase.hpp"
#include "SFML/Audio/SoundStreamState.hpp"

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
namespace
{
////////////////////////////////////////////////////////////
/// \brief State object owned by `SoundStreamState<MusicState>`
///
/// Holds all the audio-thread-touched data. Its methods
/// (`onGetData`/`onSeek`/`onLoop`) are invoked by the audio
/// callback; the `SoundStream` template guarantees the state
/// outlives any in-flight callback.
///
////////////////////////////////////////////////////////////
struct MusicState
{
    ////////////////////////////////////////////////////////////
    mutable std::mutex                loopMutex;      //!< Protects `loopSpan` and `sampleOffset`
    Music::Span<base::U64>            loopSpan;       //!< Loop range Specifier
    MusicReader&                      musicReader;    //!< The music reader
    const priv::MiniaudioSoundSource& source;         //!< Back-ref to the owning `Music` for `isLooping()`
    base::U64                         sampleOffset{}; //!< Current offset in the stream

    ////////////////////////////////////////////////////////////
    explicit MusicState(MusicReader& theMusicReader, const priv::MiniaudioSoundSource& theSource, base::U64 sampleCount) :
        loopSpan{0u, sampleCount},
        musicReader(theMusicReader),
        source(theSource)
    {
    }

    ////////////////////////////////////////////////////////////
    bool onGetData(base::Vector<base::I16>& outBuffer)
    {
        const std::lock_guard lock(loopMutex);

        // Size the output buffer to hold up to 1 second of audio samples
        outBuffer.resize(musicReader.getSampleRate() * musicReader.getChannelCount());

        base::SizeT     toFill  = outBuffer.size();
        const base::U64 loopEnd = loopSpan.offset + loopSpan.length;

        // If the loop end is enabled and imminent, request less data so we trip an `onLoop()`.
        if (source.isLooping() && (loopSpan.length != 0) && (sampleOffset <= loopEnd) && (sampleOffset + toFill > loopEnd))
            toFill = static_cast<base::SizeT>(loopEnd - sampleOffset);

        // `seekAndRead` is thread-safe
        const auto [sampleOffsetAfter, samplesRead] = musicReader.seekAndRead(sampleOffset, outBuffer.data(), toFill);

        outBuffer.resize(static_cast<base::SizeT>(samplesRead));
        sampleOffset = sampleOffsetAfter + samplesRead;

        return (samplesRead != 0) && (sampleOffset < musicReader.getSampleCount()) &&
               (sampleOffset != loopEnd || loopSpan.length == 0);
    }

    ////////////////////////////////////////////////////////////
    void onSeek(const Time timeOffset)
    {
        const std::lock_guard lock(loopMutex);
        sampleOffset = timeToSamples(musicReader.getSampleRate(), musicReader.getChannelCount(), timeOffset);
    }

    ////////////////////////////////////////////////////////////
    base::Optional<base::U64> onLoop()
    {
        const std::lock_guard lock(loopMutex);

        if (!source.isLooping())
            return base::nullOpt;

        if ((loopSpan.length != 0) && (sampleOffset == loopSpan.offset + loopSpan.length))
        {
            sampleOffset = loopSpan.offset;
            return base::makeOptional(sampleOffset);
        }

        if (sampleOffset >= musicReader.getSampleCount())
        {
            sampleOffset = 0u;
            return base::makeOptional(sampleOffset);
        }

        return base::nullOpt;
    }
};

} // namespace


////////////////////////////////////////////////////////////
struct Music::Impl
{
    SoundStreamState<MusicState> stream;

    explicit Impl(PlaybackDevice& playbackDevice, MusicReader& musicReader, const priv::MiniaudioSoundSource& source) :
        stream(playbackDevice,
               musicReader.getChannelMap(),
               musicReader.getSampleRate(),
               musicReader,
               source,
               musicReader.getSampleCount())
    {
    }

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(MusicReader);
};


////////////////////////////////////////////////////////////
Music::Music(PlaybackDevice& playbackDevice, MusicReader& musicReader, const AudioSettings& audioSettings) :
    m_impl(playbackDevice, musicReader, *this)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(MusicReader, Music, (&*m_impl), (&m_impl->stream.state().musicReader));
    applyAudioSettings(audioSettings);
}


////////////////////////////////////////////////////////////
Music::Music(PlaybackDevice& playbackDevice, MusicReader& musicReader) :
    Music{playbackDevice, musicReader, AudioSettings{}}
{
}


////////////////////////////////////////////////////////////
Music::~Music() = default;


////////////////////////////////////////////////////////////
void Music::setPlayingOffset(const Time playingOffset)
{
    m_impl->stream.setPlayingOffset(playingOffset);
}


////////////////////////////////////////////////////////////
priv::MiniaudioUtils::SoundBase& Music::getSoundBase() const
{
    return const_cast<Music*>(this)->m_impl->stream.getSoundBase();
}


////////////////////////////////////////////////////////////
Music::TimeSpan Music::getLoopPoints() const
{
    const auto& state = m_impl->stream.state();

    const auto sampleRate   = state.musicReader.getSampleRate();
    const auto channelCount = state.musicReader.getChannelCount();

    const std::lock_guard lock(state.loopMutex);

    return TimeSpan{samplesToTime(sampleRate, channelCount, state.loopSpan.offset),
                    samplesToTime(sampleRate, channelCount, state.loopSpan.length)};
}


////////////////////////////////////////////////////////////
void Music::setLoopPoints(const TimeSpan timePoints)
{
    auto& state = m_impl->stream.state();

    const auto sampleRate   = state.musicReader.getSampleRate();
    const auto channelCount = state.musicReader.getChannelCount();

    SFML_BASE_ASSERT(channelCount > 0u);
    SFML_BASE_ASSERT(sampleRate > 0u);

    const auto fileSampleCount = state.musicReader.getSampleCount();

    Span<base::U64> samplePoints{timeToSamples(sampleRate, channelCount, timePoints.offset),
                                 timeToSamples(sampleRate, channelCount, timePoints.length)};

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

    if (samplePoints.offset >= fileSampleCount)
    {
        priv::err() << "LoopPoints offset val must be in range [0, Duration).";
        state.loopSpan = {0u, state.musicReader.getSampleCount()};
        return;
    }

    if (samplePoints.length == 0u)
    {
        priv::err() << "LoopPoints length val must be nonzero.";
        state.loopSpan = {0u, state.musicReader.getSampleCount()};
        return;
    }

    samplePoints.length = base::min(samplePoints.length, fileSampleCount - samplePoints.offset);

    if (samplePoints.offset == state.loopSpan.offset && samplePoints.length == state.loopSpan.length)
        return;

    const bool oldIsPlaying = isPlaying();
    const Time oldPos       = getPlayingOffset();

    stop();
    state.loopSpan = samplePoints;

    if (oldPos != Time{})
        setPlayingOffset(oldPos);

    if (oldIsPlaying)
        resume();
}


////////////////////////////////////////////////////////////
const MusicReader& Music::getMusicReader() const
{
    return m_impl->stream.state().musicReader;
}


////////////////////////////////////////////////////////////
PlaybackDevice& Music::getPlaybackDevice() const
{
    return m_impl->stream.getPlaybackDevice();
}

} // namespace sf
