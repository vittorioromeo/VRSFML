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
#include <SFML/Audio/InputSoundFile.hpp>
#include <SFML/Audio/MusicSource.hpp>
#include <SFML/Audio/MusicStream.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <mutex>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::Time samplesToTime(unsigned int sampleRate, unsigned int channelCount, std::uint64_t samples)
{
    auto position = sf::Time::Zero;

    // Make sure we don't divide by 0
    if (sampleRate != 0 && channelCount != 0)
        position = sf::microseconds(static_cast<std::int64_t>((samples * 1000000) / (channelCount * sampleRate)));

    return position;
}

////////////////////////////////////////////////////////////
[[nodiscard]] std::uint64_t timeToSamples(unsigned int sampleRate, unsigned int channelCount, sf::Time position)
{
    // Always ROUND, no unchecked truncation, hence the addition in the numerator.
    // This avoids most precision errors arising from "samples => Time => samples" conversions
    // Original rounding calculation is ((Micros * Freq * Channels) / 1000000) + 0.5
    // We refactor it to keep std::int64_t as the data type throughout the whole operation.
    return ((static_cast<std::uint64_t>(position.asMicroseconds()) * sampleRate * channelCount) + 500000) / 1000000;
}

} // namespace

namespace sf
{
////////////////////////////////////////////////////////////
struct MusicStream::Impl
{
    MusicSource*              musicSource; //!< The music source
    std::vector<std::int16_t> samples;     //!< Temporary buffer of samples
    std::recursive_mutex      mutex;       //!< Mutex protecting the data
    Span<std::uint64_t>       loopSpan;    //!< Loop Range Specifier

    explicit Impl(MusicSource& theMusic) :
    musicSource(&theMusic),

    // Resize the internal buffer so that it can contain 1 second of audio samples
    samples(musicSource->getSampleRate() * musicSource->getChannelCount()),

    // Compute the music source positions
    loopSpan{0u, musicSource->m_file->getSampleCount()}
    {
    }

    [[nodiscard]] InputSoundFile& getFile() const noexcept
    {
        return *musicSource->m_file;
    }
};


////////////////////////////////////////////////////////////
MusicStream::MusicStream(PlaybackDevice& playbackDevice, MusicSource& musicSource) :
SoundStream(playbackDevice),
m_impl(priv::makeUnique<Impl>(musicSource))
{
    SoundStream::initialize(musicSource.getChannelCount(), musicSource.getSampleRate(), musicSource.getChannelMap());

    SFML_UPDATE_LIFETIME_DEPENDANT(Music, MusicStream, m_impl->musicSource);
}


////////////////////////////////////////////////////////////
MusicStream::~MusicStream()
{
    // We must stop before destroying the file
    if (m_impl != nullptr)
    {
        stop();
    }
}


////////////////////////////////////////////////////////////
MusicStream::MusicStream(MusicStream&&) noexcept = default;


////////////////////////////////////////////////////////////
MusicStream& MusicStream::operator=(MusicStream&&) noexcept = default;


////////////////////////////////////////////////////////////
bool MusicStream::onGetData(SoundStream::Chunk& data)
{
    const std::lock_guard lock(m_impl->mutex);

    std::size_t         toFill        = m_impl->samples.size();
    std::uint64_t       currentOffset = m_impl->getFile().getSampleOffset();
    const std::uint64_t loopEnd       = m_impl->loopSpan.offset + m_impl->loopSpan.length;

    // If the loop end is enabled and imminent, request less data.
    // This will trip an "onLoop()" call from the underlying SoundStream,
    // and we can then take action.
    if (getLoop() && (m_impl->loopSpan.length != 0) && (currentOffset <= loopEnd) && (currentOffset + toFill > loopEnd))
        toFill = static_cast<std::size_t>(loopEnd - currentOffset);

    // Fill the chunk parameters
    data.samples     = m_impl->samples.data();
    data.sampleCount = static_cast<std::size_t>(m_impl->getFile().read(m_impl->samples.data(), toFill));
    currentOffset += data.sampleCount;

    // Check if we have stopped obtaining samples or reached either the EOF or the loop end point
    return (data.sampleCount != 0) && (currentOffset < m_impl->getFile().getSampleCount()) &&
           (currentOffset != loopEnd || m_impl->loopSpan.length == 0);
}


////////////////////////////////////////////////////////////
void MusicStream::onSeek(Time timeOffset)
{
    const std::lock_guard lock(m_impl->mutex);
    m_impl->getFile().seek(timeOffset);
}


////////////////////////////////////////////////////////////
std::optional<std::uint64_t> MusicStream::onLoop()
{
    // Called by underlying SoundStream so we can determine where to loop.
    const std::lock_guard lock(m_impl->mutex);
    const std::uint64_t   currentOffset = m_impl->getFile().getSampleOffset();

    if (getLoop() && (m_impl->loopSpan.length != 0) && (currentOffset == m_impl->loopSpan.offset + m_impl->loopSpan.length))
    {
        // Looping is enabled, and either we're at the loop end, or we're at the EOF
        // when it's equivalent to the loop end (loop end takes priority). Send us to loop begin
        m_impl->getFile().seek(m_impl->loopSpan.offset);
        return m_impl->getFile().getSampleOffset();
    }

    if (getLoop() && (currentOffset >= m_impl->getFile().getSampleCount()))
    {
        // If we're at the EOF, reset to 0
        m_impl->getFile().seek(0);
        return 0;
    }

    return std::nullopt;
}


////////////////////////////////////////////////////////////
MusicStream::TimeSpan MusicStream::getLoopPoints() const
{
    return TimeSpan{samplesToTime(getSampleRate(), getChannelCount(), m_impl->loopSpan.offset),
                    samplesToTime(getSampleRate(), getChannelCount(), m_impl->loopSpan.length)};
}


////////////////////////////////////////////////////////////
void MusicStream::setLoopPoints(TimeSpan timePoints)
{
    Span<std::uint64_t> samplePoints{timeToSamples(getSampleRate(), getChannelCount(), timePoints.offset),
                                     timeToSamples(getSampleRate(), getChannelCount(), timePoints.length)};

    // Check our state. This averts a divide-by-zero. GetChannelCount() is cheap enough to use often
    if (getChannelCount() == 0 || m_impl->getFile().getSampleCount() == 0)
    {
        priv::err() << "Music is not in a valid state to assign Loop Points." << priv::errEndl;
        return;
    }

    // Round up to the next even sample if needed
    samplePoints.offset += (getChannelCount() - 1);
    samplePoints.offset -= (samplePoints.offset % getChannelCount());
    samplePoints.length += (getChannelCount() - 1);
    samplePoints.length -= (samplePoints.length % getChannelCount());

    // Validate
    if (samplePoints.offset >= m_impl->getFile().getSampleCount())
    {
        priv::err() << "LoopPoints offset val must be in range [0, Duration)." << priv::errEndl;
        return;
    }

    if (samplePoints.length == 0)
    {
        priv::err() << "LoopPoints length val must be nonzero." << priv::errEndl;
        return;
    }

    // Clamp End Point
    samplePoints.length = priv::min(samplePoints.length, m_impl->getFile().getSampleCount() - samplePoints.offset);

    // If this change has no effect, we can return without touching anything
    if (samplePoints.offset == m_impl->loopSpan.offset && samplePoints.length == m_impl->loopSpan.length)
        return;

    // When we apply this change, we need to "reset" this instance and its buffer

    // Get old playing status and position
    const Status oldStatus = getStatus();
    const Time   oldPos    = getPlayingOffset();

    // Unload
    stop();

    // Set
    m_impl->loopSpan = samplePoints;

    // Restore
    if (oldPos != Time::Zero)
        setPlayingOffset(oldPos);

    // Resume
    if (oldStatus == Status::Playing)
        play();
}

} // namespace sf
