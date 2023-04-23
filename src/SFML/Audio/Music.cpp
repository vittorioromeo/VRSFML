////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2023 Laurent Gomila (laurent@sfml-dev.org)
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
#include <SFML/Audio/ALCheck.hpp>
#include <SFML/Audio/Music.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Time.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <ostream>
#include <vector>

#include <cassert>

#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace sf
{
////////////////////////////////////////////////////////////
class Music::Impl
{
public:
    Impl(Music* parent);
    ~Impl();

    [[nodiscard]] bool openFromFile(const std::filesystem::path& filename);

    [[nodiscard]] bool openFromMemory(const void* data, std::size_t sizeInBytes);

    [[nodiscard]] bool openFromStream(InputStream& stream);

    Time getDuration() const;

    Music::TimeSpan getLoopPoints() const;

    void setLoopPoints(Music::TimeSpan timePoints);

    bool onGetData(SoundStream::Chunk& data);

    void onSeek(Time timeOffset);

    std::int64_t onLoop();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Initialize the internal state after loading a new music
    ///
    ////////////////////////////////////////////////////////////
    void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Helper to convert an sf::Time to a sample position
    ///
    /// \param position Time to convert to samples
    ///
    /// \return The number of samples elapsed at the given time
    ///
    ////////////////////////////////////////////////////////////
    std::uint64_t timeToSamples(Time position) const;

    ////////////////////////////////////////////////////////////
    /// \brief Helper to convert a sample position to an sf::Time
    ///
    /// \param samples Sample count to convert to Time
    ///
    /// \return The Time position of the given sample
    ///
    ////////////////////////////////////////////////////////////
    Time samplesToTime(std::uint64_t samples) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Music*                     m_parent;
    InputSoundFile             m_file;     //!< The streamed music file
    std::vector<std::int16_t>  m_samples;  //!< Temporary buffer of samples
    std::recursive_mutex       m_mutex;    //!< Mutex protecting the data
    Music::Span<std::uint64_t> m_loopSpan; //!< Loop Range Specifier
};


////////////////////////////////////////////////////////////
Music::Impl::Impl(Music* parent) : m_parent(parent)
{
    assert(parent != nullptr);
}


////////////////////////////////////////////////////////////
Music::Impl::~Impl()
{
    m_parent->stop();
}


////////////////////////////////////////////////////////////
bool Music::Impl::openFromFile(const std::filesystem::path& filename)
{
    // First stop the music if it was already running
    m_parent->stop();

    // Open the underlying sound file
    if (!m_file.openFromFile(filename))
        return false;

    // Perform common initializations
    initialize();

    return true;
}


////////////////////////////////////////////////////////////
bool Music::Impl::openFromMemory(const void* data, std::size_t sizeInBytes)
{
    // First stop the music if it was already running
    m_parent->stop();

    // Open the underlying sound file
    if (!m_file.openFromMemory(data, sizeInBytes))
        return false;

    // Perform common initializations
    initialize();

    return true;
}


////////////////////////////////////////////////////////////
bool Music::Impl::openFromStream(InputStream& stream)
{
    // First stop the music if it was already running
    m_parent->stop();

    // Open the underlying sound file
    if (!m_file.openFromStream(stream))
        return false;

    // Perform common initializations
    initialize();

    return true;
}


////////////////////////////////////////////////////////////
Time Music::Impl::getDuration() const
{
    return m_file.getDuration();
}


////////////////////////////////////////////////////////////
Music::TimeSpan Music::Impl::getLoopPoints() const
{
    return Music::TimeSpan{samplesToTime(m_loopSpan.offset), samplesToTime(m_loopSpan.length)};
}


////////////////////////////////////////////////////////////
void Music::Impl::setLoopPoints(Music::TimeSpan timePoints)
{
    Music::Span<std::uint64_t> samplePoints{timeToSamples(timePoints.offset), timeToSamples(timePoints.length)};

    // Check our state. This averts a divide-by-zero. GetChannelCount() is cheap enough to use often
    if (m_parent->getChannelCount() == 0 || m_file.getSampleCount() == 0)
    {
        err() << "Music is not in a valid state to assign Loop Points." << std::endl;
        return;
    }

    // Round up to the next even sample if needed
    samplePoints.offset += (m_parent->getChannelCount() - 1);
    samplePoints.offset -= (samplePoints.offset % m_parent->getChannelCount());
    samplePoints.length += (m_parent->getChannelCount() - 1);
    samplePoints.length -= (samplePoints.length % m_parent->getChannelCount());

    // Validate
    if (samplePoints.offset >= m_file.getSampleCount())
    {
        err() << "LoopPoints offset val must be in range [0, Duration)." << std::endl;
        return;
    }
    if (samplePoints.length == 0)
    {
        err() << "LoopPoints length val must be nonzero." << std::endl;
        return;
    }

    // Clamp End Point
    samplePoints.length = std::min(samplePoints.length, m_file.getSampleCount() - samplePoints.offset);

    // If this change has no effect, we can return without touching anything
    if (samplePoints.offset == m_loopSpan.offset && samplePoints.length == m_loopSpan.length)
        return;

    // When we apply this change, we need to "reset" this instance and its buffer

    // Get old playing status and position
    Status oldStatus = m_parent->getStatus();
    Time   oldPos    = m_parent->getPlayingOffset();

    // Unload
    m_parent->stop();

    // Set
    m_loopSpan = samplePoints;

    // Restore
    if (oldPos != Time::Zero)
        m_parent->setPlayingOffset(oldPos);

    // Resume
    if (oldStatus == Playing)
        m_parent->play();
}


////////////////////////////////////////////////////////////
bool Music::Impl::onGetData(SoundStream::Chunk& data)
{
    std::lock_guard lock(m_mutex);

    std::size_t   toFill        = m_samples.size();
    std::uint64_t currentOffset = m_file.getSampleOffset();
    std::uint64_t loopEnd       = m_loopSpan.offset + m_loopSpan.length;

    // If the loop end is enabled and imminent, request less data.
    // This will trip an "onLoop()" call from the underlying SoundStream,
    // and we can then take action.
    if (m_parent->getLoop() && (m_loopSpan.length != 0) && (currentOffset <= loopEnd) && (currentOffset + toFill > loopEnd))
        toFill = static_cast<std::size_t>(loopEnd - currentOffset);

    // Fill the chunk parameters
    data.samples     = m_samples.data();
    data.sampleCount = static_cast<std::size_t>(m_file.read(m_samples.data(), toFill));
    currentOffset += data.sampleCount;

    // Check if we have stopped obtaining samples or reached either the EOF or the loop end point
    return (data.sampleCount != 0) && (currentOffset < m_file.getSampleCount()) &&
           (currentOffset != loopEnd || m_loopSpan.length == 0);
}


////////////////////////////////////////////////////////////
void Music::Impl::onSeek(Time timeOffset)
{
    std::lock_guard lock(m_mutex);
    m_file.seek(timeOffset);
}


////////////////////////////////////////////////////////////
std::int64_t Music::Impl::onLoop()
{
    // Called by underlying SoundStream so we can determine where to loop.
    std::lock_guard lock(m_mutex);
    std::uint64_t   currentOffset = m_file.getSampleOffset();
    if (m_parent->getLoop() && (m_loopSpan.length != 0) && (currentOffset == m_loopSpan.offset + m_loopSpan.length))
    {
        // Looping is enabled, and either we're at the loop end, or we're at the EOF
        // when it's equivalent to the loop end (loop end takes priority). Send us to loop begin
        m_file.seek(m_loopSpan.offset);
        return static_cast<std::int64_t>(m_file.getSampleOffset());
    }
    else if (m_parent->getLoop() && (currentOffset >= m_file.getSampleCount()))
    {
        // If we're at the EOF, reset to 0
        m_file.seek(0);
        return 0;
    }
    return NoLoop;
}


////////////////////////////////////////////////////////////
void Music::Impl::initialize()
{
    // Compute the music positions
    m_loopSpan.offset = 0;
    m_loopSpan.length = m_file.getSampleCount();

    // Resize the internal buffer so that it can contain 1 second of audio samples
    m_samples.resize(m_file.getSampleRate() * m_file.getChannelCount());

    // Initialize the stream
    m_parent->SoundStream::initialize(m_file.getChannelCount(), m_file.getSampleRate());
}

////////////////////////////////////////////////////////////
std::uint64_t Music::Impl::timeToSamples(Time position) const
{
    // Always ROUND, no unchecked truncation, hence the addition in the numerator.
    // This avoids most precision errors arising from "samples => Time => samples" conversions
    // Original rounding calculation is ((Micros * Freq * Channels) / 1000000) + 0.5
    // We refactor it to keep std::int64_t as the data type throughout the whole operation.
    return ((static_cast<std::uint64_t>(position.asMicroseconds()) * m_parent->getSampleRate() *
             m_parent->getChannelCount()) +
            500000) /
           1000000;
}


////////////////////////////////////////////////////////////
Time Music::Impl::samplesToTime(std::uint64_t samples) const
{
    Time position = Time::Zero;

    // Make sure we don't divide by 0
    if (m_parent->getSampleRate() != 0 && m_parent->getChannelCount() != 0)
        position = microseconds(
            static_cast<std::int64_t>((samples * 1000000) / (m_parent->getChannelCount() * m_parent->getSampleRate())));

    return position;
}


////////////////////////////////////////////////////////////
Music::Music() = default;


////////////////////////////////////////////////////////////
Music::~Music() = default;


////////////////////////////////////////////////////////////
bool Music::openFromFile(const std::filesystem::path& filename)
{
    return m_impl->openFromFile(filename);
}


////////////////////////////////////////////////////////////
bool Music::openFromMemory(const void* data, std::size_t sizeInBytes)
{
    return m_impl->openFromMemory(data, sizeInBytes);
}


////////////////////////////////////////////////////////////
bool Music::openFromStream(InputStream& stream)
{
    return m_impl->openFromStream(stream);
}


////////////////////////////////////////////////////////////
Time Music::getDuration() const
{
    return m_impl->getDuration();
}


////////////////////////////////////////////////////////////
Music::TimeSpan Music::getLoopPoints() const
{
    return m_impl->getLoopPoints();
}


////////////////////////////////////////////////////////////
void Music::setLoopPoints(TimeSpan timePoints)
{
    m_impl->setLoopPoints(timePoints);
}


////////////////////////////////////////////////////////////
bool Music::onGetData(Chunk& data)
{
    return m_impl->onGetData(data);
}


////////////////////////////////////////////////////////////
void Music::onSeek(Time timeOffset)
{
    m_impl->onSeek(timeOffset);
}


////////////////////////////////////////////////////////////
std::int64_t Music::onLoop()
{
    return m_impl->onLoop();
}


} // namespace sf
