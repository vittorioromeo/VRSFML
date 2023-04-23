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
#include <SFML/Audio/AudioDevice.hpp>
#include <SFML/Audio/SoundStream.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Sleep.hpp>

#include <cassert>
#include <cstdlib>
#include <mutex>
#include <ostream>
#include <thread>

#ifdef _MSC_VER
#pragma warning(disable : 4355) // 'this' used in base member initializer list
#endif

#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace sf
{
////////////////////////////////////////////////////////////
class SoundStream::Impl
{
public:
    Impl(SoundStream* parent);
    ~Impl();

    void play();

    void pause();

    void stop();

    unsigned int getChannelCount() const;

    unsigned int getSampleRate() const;

    SoundSource::Status getStatus() const;

    void setPlayingOffset(Time timeOffset);

    Time getPlayingOffset() const;

    void setLoop(bool loop);

    bool getLoop() const;

    void initialize(unsigned int channelCount, unsigned int sampleRate);

    void setProcessingInterval(Time interval);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Function called as the entry point of the thread
    ///
    /// This function starts the streaming loop, and returns
    /// only when the sound is stopped.
    ///
    ////////////////////////////////////////////////////////////
    void streamData();

    ////////////////////////////////////////////////////////////
    /// \brief Fill a new buffer with audio samples, and append
    ///        it to the playing queue
    ///
    /// This function is called as soon as a buffer has been fully
    /// consumed; it fills it again and inserts it back into the
    /// playing queue.
    ///
    /// \param bufferNum Number of the buffer to fill (in [0, BufferCount])
    /// \param immediateLoop Treat empty buffers as spent, and act on loops immediately
    ///
    /// \return True if the stream source has requested to stop, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool fillAndPushBuffer(unsigned int bufferNum, bool immediateLoop = false);

    ////////////////////////////////////////////////////////////
    /// \brief Fill the audio buffers and put them all into the playing queue
    ///
    /// This function is called when playing starts and the
    /// playing queue is empty.
    ///
    /// \return True if the derived class has requested to stop, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool fillQueue();

    ////////////////////////////////////////////////////////////
    /// \brief Clear all the audio buffers and empty the playing queue
    ///
    /// This function is called when the stream is stopped.
    ///
    ////////////////////////////////////////////////////////////
    void clearQueue();

    ////////////////////////////////////////////////////////////
    /// \brief Launch a new stream thread running 'streamData'
    ///
    /// This function is called when the stream is played or
    /// when the playing offset is changed.
    ///
    ////////////////////////////////////////////////////////////
    void launchStreamingThread(SoundSource::Status threadStartState);

    ////////////////////////////////////////////////////////////
    /// \brief Stop streaming and wait for 'm_thread' to join
    ///
    /// This function is called when the playback is stopped or
    /// when the sound stream is destroyed.
    ///
    ////////////////////////////////////////////////////////////
    void awaitStreamingThread();

    // NOLINTBEGIN(readability-identifier-naming)
    static constexpr unsigned int BufferCount{3};   //!< Number of audio buffers used by the streaming loop
    static constexpr unsigned int BufferRetries{2}; //!< Number of retries (excluding initial try) for onGetData()
    // NOLINTEND(readability-identifier-naming)

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SoundStream*                 m_parent;
    std::thread                  m_thread;                    //!< Thread running the background tasks
    mutable std::recursive_mutex m_threadMutex;               //!< Thread mutex
    Status                       m_threadStartState{Stopped}; //!< State the thread starts in (Playing, Paused, Stopped)
    bool                         m_isStreaming{};             //!< Streaming state (true = playing, false = stopped)
    unsigned int                 m_buffers[BufferCount]{};    //!< Sound buffers used to store temporary audio data
    unsigned int                 m_channelCount{};            //!< Number of channels (1 = mono, 2 = stereo, ...)
    unsigned int                 m_sampleRate{};              //!< Frequency (samples / second)
    std::int32_t                 m_format{};                  //!< Format of the internal sound buffers
    bool                         m_loop{};                    //!< Loop flag (true to loop, false to play once)
    std::uint64_t                m_samplesProcessed{}; //!< Number of samples processed since beginning of the stream
    std::int64_t m_bufferSeeks[BufferCount]{}; //!< If buffer is an "end buffer", holds next seek position, else NoLoop. For play offset calculation.
    Time m_processingInterval{milliseconds(10)}; //!< Interval for checking and filling the internal sound buffers.
};


////////////////////////////////////////////////////////////
SoundStream::Impl::Impl(SoundStream* parent) : m_parent(parent)
{
    assert(parent != nullptr);
}


////////////////////////////////////////////////////////////
SoundStream::Impl::~Impl()
{
    // Stop the sound if it was playing

    // Wait for the thread to join
    awaitStreamingThread();
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::initialize(unsigned int channelCount, unsigned int sampleRate)
{
    m_channelCount     = channelCount;
    m_sampleRate       = sampleRate;
    m_samplesProcessed = 0;

    {
        std::lock_guard lock(m_threadMutex);
        m_isStreaming = false;
    }

    // Deduce the format from the number of channels
    m_format = priv::AudioDevice::getFormatFromChannelCount(channelCount);

    // Check if the format is valid
    if (m_format == 0)
    {
        m_channelCount = 0;
        m_sampleRate   = 0;
        err() << "Unsupported number of channels (" << m_channelCount << ")" << std::endl;
    }
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::play()
{
    // Check if the sound parameters have been set
    if (m_format == 0)
    {
        err() << "Failed to play audio stream: sound parameters have not been initialized (call initialize() first)"
              << std::endl;
        return;
    }

    bool                isStreaming      = false;
    SoundSource::Status threadStartState = Stopped;

    {
        std::lock_guard lock(m_threadMutex);

        isStreaming      = m_isStreaming;
        threadStartState = m_threadStartState;
    }


    if (isStreaming && (threadStartState == Paused))
    {
        // If the sound is paused, resume it
        std::lock_guard lock(m_threadMutex);
        m_threadStartState = Playing;
        alCheck(alSourcePlay(m_parent->m_source));
        return;
    }
    else if (isStreaming && (threadStartState == Playing))
    {
        // If the sound is playing, stop it and continue as if it was stopped
        stop();
    }
    else if (!isStreaming && m_thread.joinable())
    {
        // If the streaming thread reached its end, let it join so it can be restarted.
        // Also reset the playing offset at the beginning.
        stop();
    }

    // Start updating the stream in a separate thread to avoid blocking the application
    launchStreamingThread(Playing);
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::pause()
{
    // Handle pause() being called before the thread has started
    {
        std::lock_guard lock(m_threadMutex);

        if (!m_isStreaming)
            return;

        m_threadStartState = Paused;
    }

    alCheck(alSourcePause(m_parent->m_source));
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::stop()
{
    // Wait for the thread to join
    awaitStreamingThread();

    // Move to the beginning
    m_parent->onSeek(Time::Zero);
}


////////////////////////////////////////////////////////////
unsigned int SoundStream::Impl::getChannelCount() const
{
    return m_channelCount;
}


////////////////////////////////////////////////////////////
unsigned int SoundStream::Impl::getSampleRate() const
{
    return m_sampleRate;
}


////////////////////////////////////////////////////////////
SoundSource::Status SoundStream::Impl::getStatus() const
{
    SoundSource::Status status = m_parent->SoundSource::getStatus();

    // To compensate for the lag between play() and alSourceplay()
    if (status == Stopped)
    {
        std::lock_guard lock(m_threadMutex);

        if (m_isStreaming)
            status = m_threadStartState;
    }

    return status;
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::setPlayingOffset(Time timeOffset)
{
    // Get old playing status
    SoundSource::Status oldStatus = getStatus();

    // Stop the stream
    stop();

    // Let the derived class update the current position
    m_parent->onSeek(timeOffset);

    // Restart streaming
    m_samplesProcessed = static_cast<std::uint64_t>(timeOffset.asSeconds() * static_cast<float>(m_sampleRate)) *
                         m_channelCount;

    if (oldStatus == Stopped)
        return;

    launchStreamingThread(oldStatus);
}


////////////////////////////////////////////////////////////
Time SoundStream::Impl::getPlayingOffset() const
{
    if (m_sampleRate && m_channelCount)
    {
        ALfloat secs = 0.f;
        alCheck(alGetSourcef(m_parent->m_source, AL_SEC_OFFSET, &secs));

        return seconds(secs + static_cast<float>(m_samplesProcessed) / static_cast<float>(m_sampleRate) /
                                  static_cast<float>(m_channelCount));
    }
    else
    {
        return Time::Zero;
    }
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::setLoop(bool loop)
{
    m_loop = loop;
}


////////////////////////////////////////////////////////////
bool SoundStream::Impl::getLoop() const
{
    return m_loop;
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::setProcessingInterval(Time interval)
{
    m_processingInterval = interval;
}

////////////////////////////////////////////////////////////
void SoundStream::Impl::streamData()
{
    bool requestStop = false;

    {
        std::lock_guard lock(m_threadMutex);

        // Check if the thread was launched Stopped
        if (m_threadStartState == Stopped)
        {
            m_isStreaming = false;
            return;
        }
    }

    // Create the buffers
    alCheck(alGenBuffers(BufferCount, m_buffers));
    for (std::int64_t& bufferSeek : m_bufferSeeks)
        bufferSeek = NoLoop;

    // Fill the queue
    requestStop = fillQueue();

    // Play the sound
    alCheck(alSourcePlay(m_parent->m_source));

    {
        std::lock_guard lock(m_threadMutex);

        // Check if the thread was launched Paused
        if (m_threadStartState == Paused)
            alCheck(alSourcePause(m_parent->m_source));
    }

    for (;;)
    {
        {
            std::lock_guard lock(m_threadMutex);
            if (!m_isStreaming)
                break;
        }

        // The stream has been interrupted!
        if (m_parent->SoundSource::getStatus() == Stopped)
        {
            if (!requestStop)
            {
                // Just continue
                alCheck(alSourcePlay(m_parent->m_source));
            }
            else
            {
                // End streaming
                std::lock_guard lock(m_threadMutex);
                m_isStreaming = false;
            }
        }

        // Get the number of buffers that have been processed (i.e. ready for reuse)
        ALint nbProcessed = 0;
        alCheck(alGetSourcei(m_parent->m_source, AL_BUFFERS_PROCESSED, &nbProcessed));

        while (nbProcessed--)
        {
            // Pop the first unused buffer from the queue
            ALuint buffer;
            alCheck(alSourceUnqueueBuffers(m_parent->m_source, 1, &buffer));

            // Find its number
            unsigned int bufferNum = 0;
            for (unsigned int i = 0; i < BufferCount; ++i)
                if (m_buffers[i] == buffer)
                {
                    bufferNum = i;
                    break;
                }

            // Retrieve its size and add it to the samples count
            if (m_bufferSeeks[bufferNum] != NoLoop)
            {
                // This was the last buffer before EOF or Loop End: reset the sample count
                m_samplesProcessed       = static_cast<std::uint64_t>(m_bufferSeeks[bufferNum]);
                m_bufferSeeks[bufferNum] = NoLoop;
            }
            else
            {
                ALint size;
                ALint bits;
                alCheck(alGetBufferi(buffer, AL_SIZE, &size));
                alCheck(alGetBufferi(buffer, AL_BITS, &bits));

                // Bits can be 0 if the format or parameters are corrupt, avoid division by zero
                if (bits == 0)
                {
                    err() << "Bits in sound stream are 0: make sure that the audio format is not corrupt "
                          << "and initialize() has been called correctly" << std::endl;

                    // Abort streaming (exit main loop)
                    std::lock_guard lock(m_threadMutex);
                    m_isStreaming = false;
                    requestStop   = true;
                    break;
                }
                else
                {
                    m_samplesProcessed += static_cast<std::uint64_t>(size / (bits / 8));
                }
            }

            // Fill it and push it back into the playing queue
            if (!requestStop)
            {
                if (fillAndPushBuffer(bufferNum))
                    requestStop = true;
            }
        }

        // Check if any error has occurred
        if (alGetLastError() != AL_NO_ERROR)
        {
            // Abort streaming (exit main loop)
            std::lock_guard lock(m_threadMutex);
            m_isStreaming = false;
            break;
        }

        // Leave some time for the other threads if the stream is still playing
        if (m_parent->SoundSource::getStatus() != Stopped)
            sleep(m_processingInterval);
    }

    // Stop the playback
    alCheck(alSourceStop(m_parent->m_source));

    // Dequeue any buffer left in the queue
    clearQueue();

    // Reset the playing position
    m_samplesProcessed = 0;

    // Delete the buffers
    alCheck(alSourcei(m_parent->m_source, AL_BUFFER, 0));
    alCheck(alDeleteBuffers(BufferCount, m_buffers));
}


////////////////////////////////////////////////////////////
bool SoundStream::Impl::fillAndPushBuffer(unsigned int bufferNum, bool immediateLoop)
{
    bool requestStop = false;

    // Acquire audio data, also address EOF and error cases if they occur
    Chunk data = {nullptr, 0};
    for (std::uint32_t retryCount = 0; !m_parent->onGetData(data) && (retryCount < BufferRetries); ++retryCount)
    {
        // Check if the stream must loop or stop
        if (!m_loop)
        {
            // Not looping: Mark this buffer as ending with 0 and request stop
            if (data.samples != nullptr && data.sampleCount != 0)
                m_bufferSeeks[bufferNum] = 0;
            requestStop = true;
            break;
        }

        // Return to the beginning or loop-start of the stream source using onLoop(), and store the result in the buffer seek array
        // This marks the buffer as the "last" one (so that we know where to reset the playing position)
        m_bufferSeeks[bufferNum] = m_parent->onLoop();

        // If we got data, break and process it, else try to fill the buffer once again
        if (data.samples != nullptr && data.sampleCount != 0)
            break;

        // If immediateLoop is specified, we have to immediately adjust the sample count
        if (immediateLoop && (m_bufferSeeks[bufferNum] != NoLoop))
        {
            // We just tried to begin preloading at EOF or Loop End: reset the sample count
            m_samplesProcessed       = static_cast<std::uint64_t>(m_bufferSeeks[bufferNum]);
            m_bufferSeeks[bufferNum] = NoLoop;
        }

        // We're a looping sound that got no data, so we retry onGetData()
    }

    // Fill the buffer if some data was returned
    if (data.samples && data.sampleCount)
    {
        unsigned int buffer = m_buffers[bufferNum];

        // Fill the buffer
        auto size = static_cast<ALsizei>(data.sampleCount * sizeof(std::int16_t));
        alCheck(alBufferData(buffer, m_format, data.samples, size, static_cast<ALsizei>(m_sampleRate)));

        // Push it into the sound queue
        alCheck(alSourceQueueBuffers(m_parent->m_source, 1, &buffer));
    }
    else
    {
        // If we get here, we most likely ran out of retries
        requestStop = true;
    }

    return requestStop;
}


////////////////////////////////////////////////////////////
bool SoundStream::Impl::fillQueue()
{
    // Fill and enqueue all the available buffers
    bool requestStop = false;
    for (unsigned int i = 0; (i < BufferCount) && !requestStop; ++i)
    {
        // Since no sound has been loaded yet, we can't schedule loop seeks preemptively,
        // So if we start on EOF or Loop End, we let fillAndPushBuffer() adjust the sample count
        if (fillAndPushBuffer(i, (i == 0)))
            requestStop = true;
    }

    return requestStop;
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::clearQueue()
{
    // Get the number of buffers still in the queue
    ALint nbQueued;
    alCheck(alGetSourcei(m_parent->m_source, AL_BUFFERS_QUEUED, &nbQueued));

    // Dequeue them all
    ALuint buffer;
    for (ALint i = 0; i < nbQueued; ++i)
        alCheck(alSourceUnqueueBuffers(m_parent->m_source, 1, &buffer));
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::launchStreamingThread(SoundSource::Status threadStartState)
{
    {
        std::lock_guard lock(m_threadMutex);
        m_isStreaming      = true;
        m_threadStartState = threadStartState;
    }

    assert(!m_thread.joinable());
    m_thread = std::thread(&SoundStream::Impl::streamData, this);
}


////////////////////////////////////////////////////////////
void SoundStream::Impl::awaitStreamingThread()
{
    // Request the thread to join
    {
        std::lock_guard lock(m_threadMutex);
        m_isStreaming = false;
    }

    if (m_thread.joinable())
        m_thread.join();
}


////////////////////////////////////////////////////////////
SoundStream::~SoundStream() = default;


////////////////////////////////////////////////////////////
void SoundStream::play()
{
    m_impl->play();
}


////////////////////////////////////////////////////////////
void SoundStream::pause()
{
    m_impl->pause();
}


////////////////////////////////////////////////////////////
void SoundStream::stop()
{
    m_impl->stop();
}


////////////////////////////////////////////////////////////
unsigned int SoundStream::getChannelCount() const
{
    return m_impl->getChannelCount();
}


////////////////////////////////////////////////////////////
unsigned int SoundStream::getSampleRate() const
{
    return m_impl->getSampleRate();
}


////////////////////////////////////////////////////////////
SoundSource::Status SoundStream::getStatus() const
{
    return m_impl->getStatus();
}


////////////////////////////////////////////////////////////
void SoundStream::setPlayingOffset(Time timeOffset)
{
    return m_impl->setPlayingOffset(timeOffset);
}


////////////////////////////////////////////////////////////
Time SoundStream::getPlayingOffset() const
{
    return m_impl->getPlayingOffset();
}


////////////////////////////////////////////////////////////
void SoundStream::setLoop(bool loop)
{
    return m_impl->setLoop(loop);
}


////////////////////////////////////////////////////////////
bool SoundStream::getLoop() const
{
    return m_impl->getLoop();
}


////////////////////////////////////////////////////////////
SoundStream::SoundStream() : m_impl(sf::priv::makeUnique<Impl>(this))
{
}


////////////////////////////////////////////////////////////
void SoundStream::initialize(unsigned int channelCount, unsigned int sampleRate)
{
    return m_impl->initialize(channelCount, sampleRate);
}


////////////////////////////////////////////////////////////
std::int64_t SoundStream::onLoop()
{
    onSeek(Time::Zero);
    return 0;
}


////////////////////////////////////////////////////////////
void SoundStream::setProcessingInterval(Time interval)
{
    m_impl->setProcessingInterval(interval);
}


} // namespace sf
