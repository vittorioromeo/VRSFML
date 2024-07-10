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
#include <SFML/Audio/EffectProcessor.hpp>
#include <SFML/Audio/MiniaudioUtils.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundStream.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Assert.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/Macros.hpp>
#include <SFML/System/Optional.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <miniaudio.h>

#include <vector>

#include <cstring>


namespace sf
{
struct SoundStream::Impl
{
    explicit Impl(SoundStream* theOwner) : owner(theOwner)
    {
    }

    void initialize()
    {
        SFML_ASSERT(soundBase.hasValue());

        if (!soundBase->initialize(&onEnd))
            priv::err() << "Failed to initialize SoundStream::Impl" << priv::errEndl;

        // Because we are providing a custom data source, we have to provide the channel map ourselves
        if (channelMap.empty())
        {
            soundBase->getSound().engineNode.spatializer.pChannelMapIn = nullptr;
            return;
        }

        soundBase->clearSoundChannelMap();

        for (const SoundChannel channel : channelMap)
            soundBase->addToSoundChannelMap(priv::MiniaudioUtils::soundChannelToMiniaudioChannel(channel));

        soundBase->refreshSoundChannelMap();
    }

    static void onEnd(void* userData, ma_sound* soundPtr)
    {
        // Seek back to the start of the sound when it finishes playing
        auto& impl     = *static_cast<Impl*>(userData);
        impl.streaming = true;
        impl.status    = Status::Stopped;

        if (const ma_result result = ma_sound_seek_to_pcm_frame(soundPtr, 0); result != MA_SUCCESS)
            priv::MiniaudioUtils::fail("seek sound to frame 0", result);
    }

    static ma_result read(ma_data_source* dataSource, void* framesOut, ma_uint64 frameCount, ma_uint64* framesRead)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        // Try to fill our buffer with new samples if the source is still willing to stream data
        if (impl.sampleBuffer.empty() && impl.streaming)
        {
            Chunk chunk;

            impl.streaming = impl.owner->onGetData(chunk);

            if (chunk.samples && chunk.sampleCount)
            {
                impl.sampleBuffer.assign(chunk.samples, chunk.samples + chunk.sampleCount);
                impl.sampleBufferCursor = 0;
            }
        }

        // Push the samples to miniaudio
        if (!impl.sampleBuffer.empty())
        {
            // Determine how many frames we can read
            *framesRead = priv::min(frameCount, (impl.sampleBuffer.size() - impl.sampleBufferCursor) / impl.channelCount);

            const auto sampleCount = *framesRead * impl.channelCount;

            // Copy the samples to the output
            std::memcpy(framesOut,
                        impl.sampleBuffer.data() + impl.sampleBufferCursor,
                        static_cast<std::size_t>(sampleCount) * sizeof(impl.sampleBuffer[0]));

            impl.sampleBufferCursor += static_cast<std::size_t>(sampleCount);
            impl.samplesProcessed += sampleCount;

            if (impl.sampleBufferCursor >= impl.sampleBuffer.size())
            {
                impl.sampleBuffer.clear();
                impl.sampleBufferCursor = 0;

                // If we are looping and at the end of the loop, set the cursor back to the beginning of the loop
                if (!impl.streaming && impl.owner->getLoop())
                {
                    if (const auto seekPositionAfterLoop = impl.owner->onLoop())
                    {
                        impl.streaming        = true;
                        impl.samplesProcessed = *seekPositionAfterLoop;
                    }
                }
            }
        }
        else
        {
            *framesRead = 0;
        }

        return MA_SUCCESS;
    }

    static ma_result seek(ma_data_source* dataSource, ma_uint64 frameIndex)
    {
        auto& impl = *static_cast<Impl*>(dataSource);

        impl.streaming = true;
        impl.sampleBuffer.clear();
        impl.sampleBufferCursor = 0;
        impl.samplesProcessed   = frameIndex * impl.channelCount;

        const Time offset = impl.sampleRate == 0
                                ? Time::Zero
                                : seconds(static_cast<float>(frameIndex) / static_cast<float>(impl.sampleRate));

        impl.owner->onSeek(offset);
        return MA_SUCCESS;
    }

    static ma_result getFormat(ma_data_source* dataSource,
                               ma_format*      format,
                               ma_uint32*      channels,
                               ma_uint32*      sampleRate,
                               ma_channel*,
                               size_t)
    {
        const auto& impl = *static_cast<const Impl*>(dataSource);

        // If we don't have valid values yet, initialize with defaults so sound creation doesn't fail
        *format     = ma_format_s16;
        *channels   = impl.channelCount ? impl.channelCount : 1;
        *sampleRate = impl.sampleRate ? impl.sampleRate : 44100;

        return MA_SUCCESS;
    }

    static ma_result getCursor(ma_data_source* dataSource, ma_uint64* cursor)
    {
        auto& impl = *static_cast<Impl*>(dataSource);
        *cursor    = impl.channelCount ? impl.samplesProcessed / impl.channelCount : 0;

        return MA_SUCCESS;
    }

    static ma_result getLength(ma_data_source*, ma_uint64* length)
    {
        *length = 0;

        return MA_NOT_IMPLEMENTED;
    }

    static ma_result setLooping(ma_data_source* /* dataSource */, ma_bool32 /* looping */)
    {
        return MA_SUCCESS;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    static inline constexpr ma_data_source_vtable vtable{read, seek, getFormat, getCursor, getLength, setLooping, /* flags */ 0};

    sf::Optional<priv::MiniaudioUtils::SoundBase> soundBase; //!< Sound base, needs to be first member

    SoundStream*              owner;                //!< Owning `SoundStream` object
    std::vector<std::int16_t> sampleBuffer;         //!< Our temporary sample buffer
    std::size_t               sampleBufferCursor{}; //!< The current read position in the temporary sample buffer
    std::uint64_t             samplesProcessed{};   //!< Number of samples processed since beginning of the stream
    unsigned int              channelCount{};       //!< Number of channels (1 = mono, 2 = stereo, ...)
    unsigned int              sampleRate{};         //!< Frequency (samples / second)
    std::vector<SoundChannel> channelMap;           //!< The map of position in sample frame to sound channel
    bool                      streaming{true};      //!< True if we are still streaming samples from the source
    SoundSource::Status       status{SoundSource::Status::Stopped}; //!< The status
};


////////////////////////////////////////////////////////////
SoundStream::SoundStream() : m_impl(priv::makeUnique<Impl>(this))
{
}


////////////////////////////////////////////////////////////
SoundStream::~SoundStream() = default;


////////////////////////////////////////////////////////////
SoundStream::SoundStream(SoundStream&& rhs) noexcept : m_impl(SFML_MOVE(rhs.m_impl))
{
    // Update self-referential owner pointer.
    m_impl->owner = this;
}


////////////////////////////////////////////////////////////
SoundStream& SoundStream::operator=(SoundStream&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_impl = SFML_MOVE(rhs.m_impl);

        // Update self-referential owner pointer.
        m_impl->owner = this;
    }

    return *this;
}


////////////////////////////////////////////////////////////
void SoundStream::initialize(unsigned int channelCount, unsigned int sampleRate, const std::vector<SoundChannel>& channelMap)
{
    m_impl->channelCount     = channelCount;
    m_impl->sampleRate       = sampleRate;
    m_impl->channelMap       = channelMap;
    m_impl->samplesProcessed = 0;

    if (m_impl->soundBase.hasValue())
    {
        m_impl->soundBase->deinitialize();
        m_impl->initialize();

        SFML_ASSERT(m_impl->soundBase.hasValue());
        applyStoredSettings(m_impl->soundBase->getSound());
        setEffectProcessor(getEffectProcessor());
        setPlayingOffset(getPlayingOffset());
    }
}


////////////////////////////////////////////////////////////
void SoundStream::play(PlaybackDevice& playbackDevice)
{
    m_lastPlaybackDevice = &playbackDevice;

    if (!m_impl->soundBase.hasValue())
    {
        m_impl->soundBase.emplace(playbackDevice, &Impl::vtable, [](void* ptr) { static_cast<Impl*>(ptr)->initialize(); });
        m_impl->initialize();

        SFML_ASSERT(m_impl->soundBase.hasValue());
        applyStoredSettings(m_impl->soundBase->getSound());
        setEffectProcessor(getEffectProcessor());
        setPlayingOffset(getPlayingOffset());
    }

    if (m_impl->status == Status::Playing)
        setPlayingOffset(Time::Zero);

    if (const ma_result result = ma_sound_start(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("start playing sound", result);
        return;
    }

    m_impl->status = Status::Playing;
}


////////////////////////////////////////////////////////////
void SoundStream::pause()
{
    if (!m_impl->soundBase.hasValue())
        return;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("stop playing sound", result);
        return;
    }

    if (m_impl->status == Status::Playing)
        m_impl->status = Status::Paused;
}


////////////////////////////////////////////////////////////
void SoundStream::stop()
{
    if (!m_impl->soundBase.hasValue())
        return;

    if (const ma_result result = ma_sound_stop(&m_impl->soundBase->getSound()); result != MA_SUCCESS)
    {
        priv::MiniaudioUtils::fail("stop playing sound", result);
        return;
    }

    setPlayingOffset(Time::Zero);
    m_impl->status = Status::Stopped;
}


////////////////////////////////////////////////////////////
unsigned int SoundStream::getChannelCount() const
{
    return m_impl->channelCount;
}


////////////////////////////////////////////////////////////
unsigned int SoundStream::getSampleRate() const
{
    return m_impl->sampleRate;
}


////////////////////////////////////////////////////////////
std::vector<SoundChannel> SoundStream::getChannelMap() const
{
    return m_impl->channelMap;
}


////////////////////////////////////////////////////////////
SoundStream::Status SoundStream::getStatus() const
{
    return m_impl->status;
}


////////////////////////////////////////////////////////////
void SoundStream::setPlayingOffset(Time playingOffset)
{
    SoundSource::setPlayingOffset(playingOffset);

    if (!m_impl->soundBase.hasValue())
        return;

    if (m_impl->sampleRate == 0)
        return;

    if (m_impl->soundBase->getSound().pDataSource == nullptr || m_impl->soundBase->getSound().engineNode.pEngine == nullptr)
        return;

    const auto frameIndex = ma_uint64{priv::MiniaudioUtils::getFrameIndex(m_impl->soundBase->getSound(), playingOffset)};

    m_impl->streaming = true;
    m_impl->sampleBuffer.clear();
    m_impl->sampleBufferCursor = 0;
    m_impl->samplesProcessed   = frameIndex * m_impl->channelCount;

    onSeek(seconds(static_cast<float>(frameIndex) / static_cast<float>(m_impl->sampleRate)));
}


////////////////////////////////////////////////////////////
Time SoundStream::getPlayingOffset() const
{
    if (m_impl->channelCount == 0 || m_impl->sampleRate == 0)
        return Time{};

    return SoundSource::getPlayingOffset();
}


////////////////////////////////////////////////////////////
void SoundStream::setEffectProcessor(EffectProcessor effectProcessor)
{
    SoundSource::setEffectProcessor(effectProcessor);

    if (!m_impl->soundBase.hasValue())
        return;

    m_impl->soundBase->setAndConnectEffectProcessor(SFML_MOVE(effectProcessor));
}


////////////////////////////////////////////////////////////
sf::Optional<std::uint64_t> SoundStream::onLoop()
{
    onSeek(Time::Zero);
    return sf::makeOptional(std::uint64_t{0});
}


////////////////////////////////////////////////////////////
void* SoundStream::getSound() const
{
    if (!m_impl->soundBase.hasValue())
        return nullptr;

    return &m_impl->soundBase->getSound();
}

} // namespace sf
